/***************************************************************************
** CopyRight: Amlogic
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description
**
***************************************************************************/
/*
 *  Third party audio device factory
 */
#include <cstring>
#include "third_party_device_factory.h"
#include "third_party_device.h"

#define LOG_TAG "third_party_device_factory"

namespace cmcc_webrtc {

char ThirdpartyAudioDeviceFactory::_inputAudioFilename[MAX_FILENAME_LEN] = "/data/input16000.pcm";
char ThirdpartyAudioDeviceFactory::_outputAudioFilename[MAX_FILENAME_LEN] = "/data/output16000.pcm";

ThirdpartyAudioDevice* ThirdpartyAudioDeviceFactory::CreateThirdpartyAudioDevice(
    const int32_t id) {
  // Bail out here if the files aren't set.
  printf("CreateThirdpartyAudioDevice\n");
  if (strlen(_inputAudioFilename) == 0 || strlen(_outputAudioFilename) == 0) {
    printf("Was compiled with WEBRTC_DUMMY_AUDIO_PLAY_STATIC_FILE "
           "but did not set input/output files to use. Bailing out.\n");
    exit(1);
  }
  return new ThirdpartyAudioDevice(id, _inputAudioFilename, _outputAudioFilename);
}

void ThirdpartyAudioDeviceFactory::SetFilenamesToUse(
    const char* inputAudioFilename, const char* outputAudioFilename) {
#ifdef THIRD_PARTY_AUDIO_DEVICE
  assert(strlen(inputAudioFilename) < MAX_FILENAME_LEN &&
         strlen(outputAudioFilename) < MAX_FILENAME_LEN);

  // Copy the strings since we don't know the lifetime of the input pointers.
  strncpy(_inputAudioFilename, inputAudioFilename, MAX_FILENAME_LEN);
  strncpy(_outputAudioFilename, outputAudioFilename, MAX_FILENAME_LEN);
#else
  // Sanity: must be compiled with the right define to run this.
  printf("Trying to use third_party devices, but it is not be compiled.\n");
  exit(1);
#endif
}

}  // namespace cmcc_webrtc


