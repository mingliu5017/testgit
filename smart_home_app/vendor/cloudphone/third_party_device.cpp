/***************************************************************************
** CopyRight: Amlogic
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description
**
***************************************************************************/
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "third_party_device.h"

#include "sleep.h"
#include "thread_wrapper.h"

#define LOG_TAG "third_party_device"

#define VoIP_FIFO "/tmp/voip_fifo"

namespace cmcc_webrtc {

int sock;
int fifo_reader_fd;
//struct sockaddr_in peeraddr;
//socklen_t peerlen = sizeof(peeraddr);

int kRecordingFixedSampleRate = 16000;
int kRecordingNumChannels = 1;
int kPlayoutFixedSampleRate = 16000;
int kPlayoutNumChannels = 1;
int kPlayoutBufferSize = kPlayoutFixedSampleRate / 100
                         * kPlayoutNumChannels * 2;
int kRecordingBufferSize = kRecordingFixedSampleRate / 100
                           * kRecordingNumChannels * 2;

ThirdpartyAudioDevice::ThirdpartyAudioDevice(const int32_t id,
                                 const char* inputFilename,
                                 const char* outputFile):
    _ptrAudioBuffer(NULL),
    _recordingBuffer(NULL),
    _playoutBuffer(NULL),
    _recordingFramesLeft(0),
    _playoutFramesLeft(0),
    _critSect(*CriticalSectionWrapper::CreateCriticalSection()),
    _recordingBufferSizeIn10MS(0),
    _recordingFramesIn10MS(0),
    _playoutFramesIn10MS(0),
    _ptrThreadRec(NULL),
    _ptrThreadPlay(NULL),
    _recThreadID(0),
    _playThreadID(0),
    _playing(false),
    _recording(false),
    _lastCallPlayoutMillis(0),
    _lastCallRecordMillis(0),
    //_outputFile(*FileWrapper::Create()),
    //_inputFile(*FileWrapper::Create()),
    _outputFilename(outputFile),
    _inputFilename(inputFilename),
    _clock(Clock::GetRealTimeClock()) {
    printf("ThirdpartyAudioDevice(构造函数)\n");
}

ThirdpartyAudioDevice::~ThirdpartyAudioDevice() {
  //_outputFile.Flush();
  //_outputFile.CloseFile();
  //delete &_outputFile;
  //_inputFile.Flush();
  //_inputFile.CloseFile();
  snd_pcm_drain(_pcm_handle);
  snd_pcm_close(_pcm_handle);
  //delete &_inputFile;
  printf("~ThirdpartyAudioDevice(析构函数)\n");
}

int32_t ThirdpartyAudioDevice::ActiveAudioLayer(
    AudioDeviceModule::AudioLayer& audioLayer) const {
      audioLayer = AudioDeviceModule::kLinuxThirdpartyAudio;
  return 0;
}

int32_t ThirdpartyAudioDevice::Init() {

    printf("ThirdpartyAudioDevice::Init()\n");
    //Init Thirdparty device

  return 0; 
  }

int32_t ThirdpartyAudioDevice::Terminate() {
    printf("ThirdpartyAudioDevice::Terminate()\n");

    CriticalSectionScoped lock(&_critSect);

    // RECORDING
    if (_ptrThreadRec)
    {
        ThreadWrapper* tmpThread = _ptrThreadRec;
        _ptrThreadRec = NULL;
        _critSect.Leave();

        tmpThread->SetNotAlive();

        if (tmpThread->Stop())
        {
            delete tmpThread;
        }
        else
        {
            printf("ThirdpartyAudioDevice::Terminate()  failed to close down the rec audio thread\n");
        }

        _critSect.Enter();
    }

    // PLAYOUT
    if (_ptrThreadPlay)
    {
        ThreadWrapper* tmpThread = _ptrThreadPlay;
        _ptrThreadPlay = NULL;
        _critSect.Leave();

        tmpThread->SetNotAlive();

        if (tmpThread->Stop())
        {
            delete tmpThread;
        }
        else
        {
            printf("ThirdpartyAudioDevice::Terminate()  failed to close down the play audio thread");
        }

        _critSect.Enter();
    }


   return 0; 
   }

bool ThirdpartyAudioDevice::Initialized() const 
{ 
  return true;
}

int16_t ThirdpartyAudioDevice::PlayoutDevices() {
  return 1;
}

int16_t ThirdpartyAudioDevice::RecordingDevices() {
  return 1;
}

int32_t ThirdpartyAudioDevice::PlayoutDeviceName(uint16_t index,
                                            char name[kAdmMaxDeviceNameSize],
                                            char guid[kAdmMaxGuidSize]) {
  const char* kName = "thirdparty_device";
  const char* kGuid = "thirdparty_device_unique_id";
  if (index < 1) {
    memset(name, 0, kAdmMaxDeviceNameSize);
    memset(guid, 0, kAdmMaxGuidSize);
    memcpy(name, kName, strlen(kName));
    memcpy(guid, kGuid, strlen(guid));
    return 0;
  }
  return -1;
}

int32_t ThirdpartyAudioDevice::RecordingDeviceName(uint16_t index,
                                              char name[kAdmMaxDeviceNameSize],
                                              char guid[kAdmMaxGuidSize]) {
  const char* kName = "thirdparty_device";
  const char* kGuid = "thirdparty_device_unique_id";
  if (index < 1) {
    memset(name, 0, kAdmMaxDeviceNameSize);
    memset(guid, 0, kAdmMaxGuidSize);
    memcpy(name, kName, strlen(kName));
    memcpy(guid, kGuid, strlen(guid));
    return 0;
  }
  return -1;
}

int32_t ThirdpartyAudioDevice::SetPlayoutDevice(uint16_t index) {
  if (index == 0) {
    _playout_index = index;
    return 0;
  }
  return -1;
}

int32_t ThirdpartyAudioDevice::SetPlayoutDevice(
    AudioDeviceModule::WindowsDeviceType device) {
  return -1;
}

int32_t ThirdpartyAudioDevice::SetRecordingDevice(uint16_t index) {
  if (index == 0) {
    _record_index = index;
    return _record_index;
  }
  return -1;
}

int32_t ThirdpartyAudioDevice::SetRecordingDevice(
    AudioDeviceModule::WindowsDeviceType device) {
  return -1;
}

bool ThirdpartyAudioDevice::PlayInitPcm()
{
    snd_pcm_hw_params_t* params;
    int dir=0;
    unsigned int val;
	snd_pcm_uframes_t frames_get;
	unsigned int  rate_get;
	unsigned int channels_get;
//	int periodsize;
    _pcm_frames = 160;
//	periodsize = _pcm_frames *2; 
    if(snd_pcm_open(&_pcm_handle, "dmixer_auto", SND_PCM_STREAM_PLAYBACK, 0) < 0){
        perror("snd_pcm_open");
        return false;
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(_pcm_handle, params);
    snd_pcm_hw_params_set_access(_pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(_pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(_pcm_handle, params, 1);
    //snd_pcm_hw_params_set_buffer_size_near(_pcm_handle, params, &periodsize);  

    val = 16000;
    snd_pcm_hw_params_set_rate_near(_pcm_handle, params, &val, &dir);

    snd_pcm_hw_params_set_period_size_near(_pcm_handle, params, &_pcm_frames, &dir);

    if(snd_pcm_hw_params(_pcm_handle, params) < 0){
        perror("snd_pcm_hw_params");
        exit(1);
    }
	
	snd_pcm_hw_params_get_period_size(params, &frames_get, &dir);
	snd_pcm_hw_params_get_rate (params, &rate_get, &dir);
	snd_pcm_hw_params_get_channels (params, &channels_get);
	printf("set frame: %d\n",_playoutFramesIn10MS);
	printf("get frame: %lu\n",frames_get);
	printf("get rate: %u\n",rate_get);
	printf("get channel: %u\n",channels_get);
	return true;
}

int32_t ThirdpartyAudioDevice::PlayoutIsAvailable(bool& available) {
  if (_playout_index == 0) {
    available = true;
    return _playout_index;
  }
  available = false;
  return -1;
}

int32_t ThirdpartyAudioDevice::InitPlayout() {
  printf("ThirdpartyAudioDevice::InitPlayout()\n");
  if (_playing)
  {
      return -1;
  }

  PlayInitPcm();

  // Initialize the speaker (devices might have been added or removed)
  if (InitSpeaker() == -1)
  {
      printf("ThirdpartyAudioDevice  InitSpeaker() failed\n");
  }
  if (_ptrAudioBuffer)
  {
      // Update webrtc audio buffer with the selected parameters
      _ptrAudioBuffer->SetPlayoutSampleRate(kPlayoutFixedSampleRate);
      _ptrAudioBuffer->SetPlayoutChannels(kPlayoutNumChannels);
  }
  return 0;
}

bool ThirdpartyAudioDevice::PlayoutIsInitialized() const {
  return true;
}

int32_t ThirdpartyAudioDevice::RecordingIsAvailable(bool& available) {
  if (_record_index == 0) {
    available = true;
    return _record_index;
  }
  available = false;
  return -1;
}

int32_t ThirdpartyAudioDevice::InitRecording() {
  CriticalSectionScoped lock(&_critSect);

  if (_recording) {
    return -1;
  }

  if (access(VoIP_FIFO , F_OK) == 0)
  unlink(VoIP_FIFO); //删除文件

  _recordingFramesIn10MS = kRecordingFixedSampleRate/100;

  if (_ptrAudioBuffer) {
    _ptrAudioBuffer->SetRecordingSampleRate(kRecordingFixedSampleRate);
    _ptrAudioBuffer->SetRecordingChannels(kRecordingNumChannels);
  }
  return 0;
}

bool ThirdpartyAudioDevice::RecordingIsInitialized() const {
  return true;
}

int32_t ThirdpartyAudioDevice::StartPlayout() {
  printf("ThirdpartyAudioDevice::StartPlayout()\n");
  if (_playing)
  {
      return 0;
  }

  _playing = true;
  _playoutFramesLeft = 0;
  _playoutFramesIn10MS = kPlayoutFixedSampleRate/100;

  if (!_playoutBuffer)
      _playoutBuffer = new int8_t[2 *
                                  kPlayoutNumChannels *
                                  kPlayoutFixedSampleRate/100];
  if (!_playoutBuffer)
  {
    _playing = false;
    return -1;
  }

  // PLAYOUT
  const char* threadName = "webrtc_audio_module_play_thread";
  printf("ThirdpartyAudioDevice CreateThread(PlayThreadFunc)\n!");
  _ptrThreadPlay =  ThreadWrapper::CreateThread(PlayThreadFunc,
                                                this,
                                                kRealtimePriority,
                                                threadName);
  if (_ptrThreadPlay == NULL)
  {
      printf("ThirdpartyAudioDevice::StartPlayout() CreateThread(PlayThreadFunc is NULL)\n");
      _playing = false;
      delete [] _playoutBuffer;
      _playoutBuffer = NULL;
      return -1;
  }
#if 0
  if (_outputFile.OpenFile(_outputFilename.c_str(),
                           false, false, false) == -1) {
    printf("ThirdpartyAudioDevice Failed to open playout file %s!", _outputFilename.c_str());
    _playing = false;
    delete [] _playoutBuffer;
    _playoutBuffer = NULL;
    return -1;
  }
#endif
  unsigned int threadID(0);
  if (!_ptrThreadPlay->Start(threadID))
  {
      _playing = false;
      delete _ptrThreadPlay;
      _ptrThreadPlay = NULL;
      delete [] _playoutBuffer;
      _playoutBuffer = NULL;
      return -1;
  }
  _playThreadID = threadID;

  return 0;
}

int32_t ThirdpartyAudioDevice::StopPlayout() {
  {
      CriticalSectionScoped lock(&_critSect);
      _playing = false;
  }

  // stop playout thread first
  if (_ptrThreadPlay && !_ptrThreadPlay->Stop())
  {
      return -1;
  }
  else {
      delete _ptrThreadPlay;
      _ptrThreadPlay = NULL;
  }

  CriticalSectionScoped lock(&_critSect);

  _playoutFramesLeft = 0;
  delete [] _playoutBuffer;
  _playoutBuffer = NULL;
  //_outputFile.Flush();
  //_outputFile.CloseFile();
   return 0;
}

bool ThirdpartyAudioDevice::Playing()  { //const
  StartPlayout();
  return true;
}

int32_t ThirdpartyAudioDevice::StartRecording() {
  struct sockaddr_in servaddr;
  _recording = true;
  printf("ThirdpartyAudioDevice::StartRecording()\n");
  // Make sure we only create the buffer once.
  _recordingBufferSizeIn10MS = _recordingFramesIn10MS *
                               kRecordingNumChannels *
                               2;
  if (!_recordingBuffer) {
      _recordingBuffer = new int8_t[_recordingBufferSizeIn10MS];
  }
#if 0
  if (_inputFile.OpenFile(_inputFilename.c_str(), true,
                              true, false) == -1) {
    printf("ThirdpartyAudioDevice Failed to open audio input file %s!\n",
           _inputFilename.c_str());
    _recording = false;
    delete[] _recordingBuffer;
    _recordingBuffer = NULL;
    return -1;
  }
#endif
#if 0
	if((sock = socket(PF_INET, SOCK_DGRAM|SOCK_NONBLOCK, 0)) < 0){
	//if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
#endif
	// hzj
	//printf("call start......\n");
	if((mkfifo(VoIP_FIFO ,O_CREAT|O_EXCL|O_RDWR)<0)&&(errno!=EEXIST))
	{
		perror("create fifo error.");
	}
/* 打开管道 */
	fifo_reader_fd=open(VoIP_FIFO ,O_RDONLY|O_NONBLOCK,0);
	if(fifo_reader_fd ==-1)
	{
		perror("##############\nVoIP open");
	//	exit(1);	
	}

#if 0
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("bind error");
	} else {
		printf("scket bind sucess!!!\n");
	}

#endif
  const char* threadName = "webrtc_audio_module_capture_thread";
  printf("ThirdpartyAudioDevice::StartRecording() CreateThread(RecThreadFunc)\n");
  _ptrThreadRec = ThreadWrapper::CreateThread(RecThreadFunc,
                                              this,
                                              kRealtimePriority,
                                              threadName);
  if (_ptrThreadRec == NULL)
  {
      printf("ThirdpartyAudioDevice::StartRecording() CreateThread(RecThreadFunc is NULL)\n");
      _recording = false;
      delete [] _recordingBuffer;
      _recordingBuffer = NULL;
      return -1;
  }

  unsigned int threadID(0);
  if (!_ptrThreadRec->Start(threadID))
  {
      _recording = false;
      delete _ptrThreadRec;
      _ptrThreadRec = NULL;
      delete [] _recordingBuffer;
      _recordingBuffer = NULL;
      return -1;
  }
  _recThreadID = threadID;

  return 0;
}


int32_t ThirdpartyAudioDevice::StopRecording() {
  {
    CriticalSectionScoped lock(&_critSect);
    _recording = false;
  }
  if(fifo_reader_fd>0)
  {
   printf("StopRecording....close fifo_reader_fd\n");
   close(fifo_reader_fd); //关闭管道
   unlink(VoIP_FIFO); //删除文件
	fifo_reader_fd = 0;
  }
 


  if (_ptrThreadRec && !_ptrThreadRec->Stop())
  {
      return -1;
  }
  else {
      delete _ptrThreadRec;
      _ptrThreadRec = NULL;
  }

  CriticalSectionScoped lock(&_critSect);
  _recordingFramesLeft = 0;
  if (_recordingBuffer)
  {
      delete [] _recordingBuffer;
      _recordingBuffer = NULL;
  }
  return 0;
}

bool ThirdpartyAudioDevice::Recording() const {
  return _recording;
}

int32_t ThirdpartyAudioDevice::SetAGC(bool enable) { return -1; }

bool ThirdpartyAudioDevice::AGC() const { return false; }

int32_t ThirdpartyAudioDevice::SetWaveOutVolume(uint16_t volumeLeft,
                                           uint16_t volumeRight) {
  return -1;
}

int32_t ThirdpartyAudioDevice::WaveOutVolume(uint16_t& volumeLeft,
                                        uint16_t& volumeRight) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::InitSpeaker() 
{ 
  int ret = -2;
  printf("ThirdpartyAudioDevice::InitSpeaker()\n");
  CriticalSectionScoped lock(&_critSect);

  if (_playing)
  {
    printf("ThirdpartyAudioDevice::InitSpeaker _playing is true, return -1\n");
    return -1;
  }

  //ret =_mixerManager.OpenSpeaker(devName);
  ret = 1;

  return ret;
}

bool ThirdpartyAudioDevice::SpeakerIsInitialized() const 
{ 
  printf("ThirdpartyAudioDevice::SpeakerIsInitialized()\n");
  return true; 
}

int32_t ThirdpartyAudioDevice::InitMicrophone() 
{ 
  printf("ThirdpartyAudioDevice::InitMicrophone()\n");
  return 0; 
}

bool ThirdpartyAudioDevice::MicrophoneIsInitialized() const { return true; }

int32_t ThirdpartyAudioDevice::SpeakerVolumeIsAvailable(bool& available) {
  return -1;
}

int32_t ThirdpartyAudioDevice::SetSpeakerVolume(uint32_t volume) { return -1; }

int32_t ThirdpartyAudioDevice::SpeakerVolume(uint32_t& volume) const { return -1; }

int32_t ThirdpartyAudioDevice::MaxSpeakerVolume(uint32_t& maxVolume) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::MinSpeakerVolume(uint32_t& minVolume) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::SpeakerVolumeStepSize(uint16_t& stepSize) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::MicrophoneVolumeIsAvailable(bool& available) {
  return -1;
}

int32_t ThirdpartyAudioDevice::SetMicrophoneVolume(uint32_t volume) { return -1; }

int32_t ThirdpartyAudioDevice::MicrophoneVolume(uint32_t& volume) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::MaxMicrophoneVolume(uint32_t& maxVolume) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::MinMicrophoneVolume(uint32_t& minVolume) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::MicrophoneVolumeStepSize(uint16_t& stepSize) const {
  return -1;
}

int32_t ThirdpartyAudioDevice::SpeakerMuteIsAvailable(bool& available) { return -1; }

int32_t ThirdpartyAudioDevice::SetSpeakerMute(bool enable) { return -1; }

int32_t ThirdpartyAudioDevice::SpeakerMute(bool& enabled) const { return -1; }

int32_t ThirdpartyAudioDevice::MicrophoneMuteIsAvailable(bool& available) {
  return -1;
}

int32_t ThirdpartyAudioDevice::SetMicrophoneMute(bool enable) { return -1; }

int32_t ThirdpartyAudioDevice::MicrophoneMute(bool& enabled) const { return -1; }

int32_t ThirdpartyAudioDevice::MicrophoneBoostIsAvailable(bool& available) {
  return -1;
}

int32_t ThirdpartyAudioDevice::SetMicrophoneBoost(bool enable) { return -1; }

int32_t ThirdpartyAudioDevice::MicrophoneBoost(bool& enabled) const { return -1; }

int32_t ThirdpartyAudioDevice::StereoPlayoutIsAvailable(bool& available) {

    printf("ThirdpartyAudioDevice::StereoPlayoutIsAvailable()\n");
    CriticalSectionScoped lock(&_critSect);

    // Save rec states and the number of rec channels
    bool playing = _playing;

    available = false;


    if (InitPlayout() == 0)
    {
        available = true;
    }

    // Stop/uninitialize recording
    StopPlayout();

    if (playing)
    {
        printf("ThirdpartyAudioDevice::StereoPlayoutIsAvailable(), playing is true, StartPlayout()\n");
        StartPlayout();
    }

    return 0;
}
int32_t ThirdpartyAudioDevice::SetStereoPlayout(bool enable) {
  return 0;
}

int32_t ThirdpartyAudioDevice::StereoPlayout(bool& enabled) const {
  enabled = true;
  return 0;
}

int32_t ThirdpartyAudioDevice::StereoRecordingIsAvailable(bool& available) {
  available = true;
  return 0;
}

int32_t ThirdpartyAudioDevice::SetStereoRecording(bool enable) {
  return 0;
}

int32_t ThirdpartyAudioDevice::StereoRecording(bool& enabled) const {
  enabled = true;
  return 0;
}

int32_t ThirdpartyAudioDevice::SetPlayoutBuffer(
  const AudioDeviceModule::BufferType type,
  uint16_t sizeMS) {
  _playBufType = type;

  return 0;
}

int32_t ThirdpartyAudioDevice::PlayoutBuffer(AudioDeviceModule::BufferType& type,
                                        uint16_t& sizeMS) const {
  type = _playBufType;
  return 0;
}

int32_t ThirdpartyAudioDevice::PlayoutDelay(uint16_t& delayMS) const {
  return 0;
}

int32_t ThirdpartyAudioDevice::RecordingDelay(uint16_t& delayMS) const { return -1; }

int32_t ThirdpartyAudioDevice::CPULoad(uint16_t& load) const { return -1; }

bool ThirdpartyAudioDevice::PlayoutWarning() const { return false; }

bool ThirdpartyAudioDevice::PlayoutError() const { return false; }

bool ThirdpartyAudioDevice::RecordingWarning() const { return false; }

bool ThirdpartyAudioDevice::RecordingError() const { return false; }

void ThirdpartyAudioDevice::ClearPlayoutWarning() {}

void ThirdpartyAudioDevice::ClearPlayoutError() {}

void ThirdpartyAudioDevice::ClearRecordingWarning() {}

void ThirdpartyAudioDevice::ClearRecordingError() {}

void ThirdpartyAudioDevice::AttachAudioBuffer(AudioDeviceBuffer* audioBuffer) {
  CriticalSectionScoped lock(&_critSect);

  _ptrAudioBuffer = audioBuffer;

  // Inform the AudioBuffer about default settings for this implementation.
  // Set all values to zero here since the actual settings will be done by
  // InitPlayout and InitRecording later.
  _ptrAudioBuffer->SetRecordingSampleRate(0);
  _ptrAudioBuffer->SetPlayoutSampleRate(0);
  _ptrAudioBuffer->SetRecordingChannels(0);
  _ptrAudioBuffer->SetPlayoutChannels(0);
}

bool ThirdpartyAudioDevice::PlayThreadFunc(void* pThis)
{
    return (static_cast<ThirdpartyAudioDevice*>(pThis)->PlayThreadProcess());
}

bool ThirdpartyAudioDevice::RecThreadFunc(void* pThis)
{
    return (static_cast<ThirdpartyAudioDevice*>(pThis)->RecThreadProcess());
}

bool ThirdpartyAudioDevice::PlayThreadProcess()
{
	int rc;
	int time_delay;
    if(!_playing)
    {
        printf("ThirdpartyAudioDevice _playing if false\n");
        return false;
    }

    uint64_t currentTime = _clock->CurrentNtpInMilliseconds();
    _critSect.Enter();
	time_delay=currentTime - _lastCallPlayoutMillis;
    if (_lastCallPlayoutMillis == 0 ||
		time_delay >= 9)
    {
        _critSect.Leave();
        _ptrAudioBuffer->RequestPlayoutData(_playoutFramesIn10MS);
        _critSect.Enter();

        _playoutFramesLeft = _ptrAudioBuffer->GetPlayoutData(_playoutBuffer);

        assert(_playoutFramesLeft == _playoutFramesIn10MS);
        //if (_outputFile.Open()) {
			rc = snd_pcm_writei(_pcm_handle, _playoutBuffer, kPlayoutBufferSize/2);
			if (rc == -EPIPE) {
				fprintf(stderr, "underrun occurred\n");
				snd_pcm_prepare(_pcm_handle);
			} else if (rc < 0) {
				fprintf(stderr, "%s: error from writei: %s\n",__func__,snd_strerror(rc));
			}
          //_outputFile.Write(_playoutBuffer, kPlayoutBufferSize);
          //_outputFile.Flush();
        //}
        _lastCallPlayoutMillis = currentTime;
    }
    _playoutFramesLeft = 0;
    _critSect.Leave();
    SleepMs(9 - (_clock->CurrentNtpInMilliseconds() - currentTime));
    return true;
}

bool ThirdpartyAudioDevice::RecThreadProcess()
{
	int size = 0;
	int nread;

	short buf[160];
	short dis_tab[320];
    if (!_recording)
        return false;

    uint64_t currentTime = _clock->CurrentNtpInMilliseconds();
    _critSect.Enter();

    if (_lastCallRecordMillis == 0 ||
        currentTime - _lastCallRecordMillis >= 9) 
	{
	// hzj
		memset(buf,0,kRecordingBufferSize);
		if((nread=read(fifo_reader_fd, buf,kRecordingBufferSize))==-1)
		{
			if(errno==EAGAIN)
				printf("no data yet\n");
		}
		//printf(".....%d\n", nread);
		memcpy(_recordingBuffer, buf, kRecordingBufferSize);
        _ptrAudioBuffer->SetRecordedBuffer(_recordingBuffer,
                                        _recordingFramesIn10MS);
        _lastCallRecordMillis = currentTime;
        _critSect.Leave();
        _ptrAudioBuffer->DeliverRecordedData();
        _critSect.Enter();
      //}
    }

    _critSect.Leave();
    SleepMs(9 - (_clock->CurrentNtpInMilliseconds() - currentTime));
    return true;
}

}  // namespace cmcc_webrtc