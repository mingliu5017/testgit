/***************************************************************************
** CopyRight: Amlogic
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description
**
***************************************************************************/
#define LOG_TAG "cloudphone"

#include "cloudphone.h"
#include <iostream>
#include "voipengine_api.h"
#include <stdio.h>
using namespace std;

cloudphone_OnNetConnected_fun _cloudphone_OnNetConnected_fun = NULL;
cloudphone_OnNetConnetionFailed_fun _cloudphone_OnNetConnetionFailed_fun = NULL;
cloudphone_OnUserLoginSucceed_fun _cloudphone_OnUserLoginSucceed_fun = NULL;
cloudphone_OnUserLoginFailed_fun _cloudphone_OnUserLoginFailed_fun = NULL;
cloudphone_OnUserLogout_fun _cloudphone_OnUserLogout_fun = NULL;
cloudphone_OnRecvCall_fun _cloudphone_OnRecvCall_fun = NULL;
cloudphone_OnRecvAnswer_fun _cloudphone_OnRecvAnswer_fun = NULL;
cloudphone_OnRecvRing_fun _cloudphone_OnRecvRing_fun = NULL;
cloudphone_OnResumeSession_fun _cloudphone_OnResumeSession_fun = NULL;
cloudphone_OnRecvHangup_fun _cloudphone_OnRecvHangup_fun = NULL;
cloudphone_OnRecvHangupAck_fun _cloudphone_OnRecvHangupAck_fun = NULL;
cloudphone_OnRecvReinvite_fun _cloudphone_OnRecvReinvite_fun = NULL;
cloudphone_OnRecvMessage_fun _cloudphone_OnRecvMessage_fun = NULL;
cloudphone_OnRecvSSRCChanged_fun _cloudphone_OnRecvSSRCChanged_fun = NULL;
cloudphone_OnRecvCSRCChanged_fun _cloudphone_OnRecvCSRCChanged_fun = NULL;
cloudphone_OnCameraStatusChanged_fun _cloudphone_OnCameraStatusChanged_fun = NULL;
cloudphone_OnCallForwarding_fun _cloudphone_OnCallForwarding_fun = NULL;

cloudphone_OnBFCPSendStop_fun _cloudphone_OnBFCPSendStop_fun = NULL;
cloudphone_OnBFCPSendStart_fun _cloudphone_OnBFCPSendStart_fun = NULL;
cloudphone_OnBFCPRecvStart_fun _cloudphone_OnBFCPRecvStart_fun = NULL;
cloudphone_OnBFCPRecvStop_fun _cloudphone_OnBFCPRecvStop_fun = NULL;
cloudphone_OnBFCPMessage_fun _cloudphone_OnBFCPMessage_fun = NULL;

char log_file_name[20]={0};

char deviceId[18]={0};
std::string Agent = "voipsdk/LinuxA113migu";
std::string appKey =  "ii2eygys6vxigmhy";
std::string appSecret = "gs0f6nffsrbbrwe7";

class MySignalingObserver: public SignalingObserver{

public:
  MySignalingObserver(){};

  virtual ~MySignalingObserver(){};
	void OnNetConnected(){
        cout << "====================================================" << endl;
        cout << "OnNetConnected()---- 网络连接成功" << endl;
        cout << "====================================================" << endl;
		if (_cloudphone_OnNetConnected_fun == NULL) 
			return ;
		_cloudphone_OnNetConnected_fun();
	}
	void OnNetConnetionFailed(int errorCode){
        cout << "====================================================" << endl;
        cout << "OnNetConnetionFailed()---- 网络连接失败" << endl;
        cout << "====================================================" << endl;
		if (_cloudphone_OnNetConnetionFailed_fun == NULL)
			return;
		_cloudphone_OnNetConnetionFailed_fun(errorCode);
	}
	void OnUserLoginSucceed(const char*user){
		GetSDKVersion();
        cout << "====================================================" << endl;
        cout << "OnUserLoginSucceed()---- 注册成功" << endl;
        cout << "====================================================" << endl;
		if (_cloudphone_OnUserLoginSucceed_fun == NULL)
			return;
		_cloudphone_OnUserLoginSucceed_fun(user);
	}
	void OnUserLoginFailed(const char*user, int errorCode, const char* reason){
		int ret;
		GetSDKVersion();
        cout << "====================================================" << endl;
        cout << "OnUserLoginFailed()---- 注册失败, reason:" << reason << endl;
		//if(strcmp(reason,"504")==0)
		if(0)
		{
			ret = cloudphoneGetAccount(NULL);
			cout << "cloudphoneGetAccount()---- ret:" << ret << endl;
		}
        cout << "====================================================" << endl;
		if (_cloudphone_OnUserLoginFailed_fun == NULL)
			return;
		_cloudphone_OnUserLoginFailed_fun(user, errorCode, reason);
	}
	void OnUserLogout(const char*user){
        cout << "====================================================" << endl;
        cout << "OnUserLogout()---- 用户已注销" << endl;
        cout << "====================================================" << endl;
		if (_cloudphone_OnUserLogout_fun == NULL)
			return;
		_cloudphone_OnUserLogout_fun(user);
	}
	void OnRecvCall(int session, const char* from, const char* displayname, const char* to, int callType){
		GetSDKVersion();
		std::string phoneName;
		int ret = PhoneNumber2Name(displayname,phoneName);
		cout << "====================================================" << endl;
		cout << "OnRecvCall()---- 收到来自 "<< displayname << "("<< phoneName <<") 的电话, ret=" << ret << endl;
		cout << "====================================================" << endl;
		if (_cloudphone_OnRecvCall_fun == NULL)
			return ;

		if(ret == 0){
			_cloudphone_OnRecvCall_fun(session, from, phoneName.c_str(), to, callType);
		}else{
			_cloudphone_OnRecvCall_fun(session, from, displayname, to, callType);
		}
	}
    void OnRecvAnswer(int session, const char* from, const char* displayname, const char* to, int callType){
        cout << "====================================================" << endl;
        cout << "OnRecvAnswer("<<displayname<<")---- 已接听" << endl; 
        cout << "====================================================" << endl;
		if (_cloudphone_OnRecvAnswer_fun == NULL)
			return;
		_cloudphone_OnRecvAnswer_fun(session, from, displayname, to, callType);
	}

    void OnRecvRing(int session, const char* from, const char* displayname, const char* to, int EarlyMedia){//0 no media, 1 server media
        cout << "====================================================" << endl;
        cout << "OnRecvRing()---- 对方已经响铃, media type:"<< EarlyMedia << endl;
        cout << "====================================================" << endl;
        //EarlyMedia is 1, 播放服务器回铃音，EarlyMedia is 0, 播放本地回铃音
		if (_cloudphone_OnRecvRing_fun == NULL)
			return;
		_cloudphone_OnRecvRing_fun(session, from, displayname, to, EarlyMedia);
	}
	void OnResumeSession(int sesion){
        cout << "OnResumeSession()----" << endl;
		if (_cloudphone_OnResumeSession_fun == NULL)
			return;
		_cloudphone_OnResumeSession_fun(sesion);
	}
	void OnRecvHangup(int session, int errorCode, const char* reason){
        cout << "====================================================" << endl;
        cout << "OnRecvHangup()---- 呼叫被挂断" << endl;
        cout << "====================================================" << endl;
		if (_cloudphone_OnRecvHangup_fun == NULL)
			return;
		_cloudphone_OnRecvHangup_fun(session, errorCode, reason);
	}
	void OnRecvHangupAck(int session, int errorCode, const char* reason){
        cout << "====================================================" << endl;
        cout << "OnRecvHangupAck()---- 收到挂断反馈\n" << endl;
        cout << "====================================================" << endl;
		if (_cloudphone_OnRecvHangupAck_fun == NULL)
			return;
		_cloudphone_OnRecvHangupAck_fun(session, errorCode, reason);
	}
	void OnRecvReinvite(int session, const char* from, const char* displayname, const char* to, int callType){
        cout << "OnRecvReinvite()----" << endl;
		if (_cloudphone_OnRecvReinvite_fun == NULL)
			return;
		_cloudphone_OnRecvReinvite_fun(session, from, displayname, to, callType);
	}
	void OnRecvMessage(int session, const char* from, const char* displayname, const char* to, const char* content){
        cout << "OnRecvMessage()----" << endl;
		if (_cloudphone_OnRecvMessage_fun == NULL)
			return;
		_cloudphone_OnRecvMessage_fun(session, from, displayname, to, content);
	}
	void OnRecvSSRCChanged(int session, int channel, unsigned int ssrc){
        cout << "OnRecvSSRCChanged()----" << endl;
		if (_cloudphone_OnRecvSSRCChanged_fun == NULL)
			return;
		_cloudphone_OnRecvSSRCChanged_fun(session, channel, ssrc);
	}
	void OnRecvCSRCChanged(int session, int channel, unsigned int csrc){
        cout << "OnRecvCSRCChanged()----" << endl;
		if (_cloudphone_OnRecvCSRCChanged_fun == NULL)
			return;
		_cloudphone_OnRecvCSRCChanged_fun(session, channel, csrc);
	}
	void OnCameraStatusChanged(int session, int id, int status){
        cout << "OnCameraStatusChanged()----" << endl;        
		if (_cloudphone_OnCameraStatusChanged_fun == NULL)
			return;
		_cloudphone_OnCameraStatusChanged_fun(session, id, status);
	}
	void OnCallForwarding(int session, const char* from, const char* displayname, const char* to, int callType){
        cout << "OnCallForwarding()----" << endl;
		if (_cloudphone_OnCallForwarding_fun == NULL)
			return;
		_cloudphone_OnCallForwarding_fun(session, from, displayname, to, callType);
        
	}
	void OnBFCPSendStop(int session){
		if (_cloudphone_OnBFCPSendStop_fun == NULL)
			return;
		_cloudphone_OnBFCPSendStop_fun(session);
	}
	void OnBFCPSendStart(int session){
		if (_cloudphone_OnBFCPSendStart_fun == NULL)
			return;
		_cloudphone_OnBFCPSendStart_fun(session);
	}
	void OnBFCPRecvStart(int session, int channleID){
		if (_cloudphone_OnBFCPRecvStart_fun == NULL)
			return;
		_cloudphone_OnBFCPRecvStart_fun(session, channleID);
	}
	void OnBFCPRecvStop(int session, int channleID){
		if (_cloudphone_OnBFCPRecvStop_fun == NULL)
			return;
		_cloudphone_OnBFCPRecvStop_fun(session, channleID);
	}
	void OnBFCPMessage(int session, const char* obj, const char* action){       
		if (_cloudphone_OnBFCPMessage_fun == NULL)
			return;
		_cloudphone_OnBFCPMessage_fun(session, obj, action);
	}
};

class mytraceCallback:public externalTraceCallback{
public:
  mytraceCallback(){};
  virtual ~mytraceCallback(){};
  virtual void Print(int level, const char* message, int length){
		cout << "Trace,level:"<< level <<",m:"<< message << endl;
     }
};
class myAudioStatisticsObserver:public externalAudioStatisticsObserver{
public:
    myAudioStatisticsObserver(){};
    virtual ~myAudioStatisticsObserver(){};
    virtual void AudioStatistics(int session,
                                 int fractionLost,
                                 int cumulativeLost,
                                 int extendedMax,
                                 int jitterSamples,
                                 int rttMs,
                                 int bytesSent,
                                 int packetsSent,
                                 int bytesReceived,
                                 int packetsReceived){
        cout << "audio,session:"<<session<<",lost:"<<fractionLost<<",jitter:"<<jitterSamples<<",rtt:"<<rttMs<<",bytesReceived:"<<packetsReceived << endl;
    }
};

void cloudphone_OnNetConnected_fun_register(cloudphone_OnNetConnected_fun func) {
	_cloudphone_OnNetConnected_fun = func;
}
void cloudphone_OnNetConnetionFailed_fun_register(cloudphone_OnNetConnetionFailed_fun func) {
	_cloudphone_OnNetConnetionFailed_fun = func;
}
void cloudphone_OnUserLoginSucceed_fun_register(cloudphone_OnUserLoginSucceed_fun func) {
	_cloudphone_OnUserLoginSucceed_fun = func;
}
void cloudphone_OnUserLoginFailed_fun_register(cloudphone_OnUserLoginFailed_fun func) {
	_cloudphone_OnUserLoginFailed_fun = func;
}
void cloudphone_OnUserLogout_fun_register(cloudphone_OnUserLogout_fun func) {
	_cloudphone_OnUserLogout_fun = func;
}
void cloudphone_OnRecvCall_fun_register(cloudphone_OnRecvCall_fun func) {
	_cloudphone_OnRecvCall_fun = func;
}
void cloudphone_OnRecvAnswer_fun_register(cloudphone_OnRecvAnswer_fun func) {
	_cloudphone_OnRecvAnswer_fun = func;
}
void cloudphone_OnRecvRing_fun_register(cloudphone_OnRecvRing_fun func) {
	_cloudphone_OnRecvRing_fun = func;
}
void cloudphone_OnResumeSession_fun_register(cloudphone_OnResumeSession_fun func) {
	_cloudphone_OnResumeSession_fun = func;
}
void cloudphone_OnRecvHangup_fun_register(cloudphone_OnRecvHangup_fun func) {
	_cloudphone_OnRecvHangup_fun = func;
}
void cloudphone_OnRecvHangupAck_fun_register(cloudphone_OnRecvHangupAck_fun func) {
	_cloudphone_OnRecvHangupAck_fun = func;
}
void cloudphone_OnRecvReinvite_fun_register(cloudphone_OnRecvReinvite_fun func) {
	_cloudphone_OnRecvReinvite_fun = func;
}
void cloudphone_OnRecvMessage_fun_register(cloudphone_OnRecvMessage_fun func) {
	_cloudphone_OnRecvMessage_fun = func;
}
void cloudphone_OnRecvSSRCChanged_fun_register(cloudphone_OnRecvSSRCChanged_fun func) {
	_cloudphone_OnRecvSSRCChanged_fun = func;
}
void cloudphone_OnRecvCSRCChanged_fun_register(cloudphone_OnRecvCSRCChanged_fun func) {
	_cloudphone_OnRecvCSRCChanged_fun = func;
}
void cloudphone_OnCameraStatusChanged_fun_register(cloudphone_OnCameraStatusChanged_fun func) {
	_cloudphone_OnCameraStatusChanged_fun = func;
}
void cloudphone_OnCallForwarding_fun_register(cloudphone_OnCallForwarding_fun func) {
	_cloudphone_OnCallForwarding_fun = func;
}
void cloudphone_OnBFCPSendStop_fun_register(cloudphone_OnBFCPSendStop_fun func) {
	_cloudphone_OnBFCPSendStop_fun = func;
}
void cloudphone_OnBFCPSendStart_fun_register(cloudphone_OnBFCPSendStart_fun func) {
	_cloudphone_OnBFCPSendStart_fun = func;
}
void cloudphone_OnBFCPRecvStart_fun_register(cloudphone_OnBFCPRecvStart_fun func) {
	_cloudphone_OnBFCPRecvStart_fun = func;
}
void cloudphone_OnBFCPRecvStop_fun_register(cloudphone_OnBFCPRecvStop_fun func) {
	_cloudphone_OnBFCPRecvStop_fun = func;
}
void cloudphone_OnBFCPMessage_fun_register(cloudphone_OnBFCPMessage_fun func) {
	_cloudphone_OnBFCPMessage_fun = func;
}

int cloudphoneInit(char* sys_version)
#if 0
{
    //cout << "begin test audio" << endl;
    int fd; 
	int ret;
    CreateEngine();
    mytraceCallback* trace = new mytraceCallback();
    RegisterTraceCallback(trace, LOG_FILE_PATH);//with log file
    MySignalingObserver* signalObserver = new MySignalingObserver();
    RegisterSignalingObserver(signalObserver);
	
    return 0;
}
#else
{
    //cout << "begin test audio" << endl;
    int fd; 
	int ret;

    fd = open("/data/btMac", O_RDWR);
    if (fd < 0) {
		printf("open btmac file err!\n");
		return -1;
	}
    else {
        memset(deviceId, '\0', sizeof(deviceId));
        ret=read(fd, deviceId, sizeof(deviceId));
		deviceId[17]='\0';
        printf("ret:%d,get bt mac: %s\n", ret,deviceId);
        close(fd);
    }

	sprintf(log_file_name,"/data/migu_31123_%s.log",deviceId);
    CreateEngine();
    mytraceCallback* trace = new mytraceCallback();
    RegisterTraceCallback(trace, log_file_name );//with log file
    MySignalingObserver* signalObserver = new MySignalingObserver();
    RegisterSignalingObserver(signalObserver);
	if (sys_version != NULL)
	{
		//SetSystemImageVersion(sys_version);
	}
	else
	{
		//SetSystemImageVersion("NULL");
	}
	
    return 0;
}

#endif

int cloudphoneGetAccount(char* homeserver_id)
{
    //cout << "begin test audio" << endl;
    int fd; 
	int ret;

    fd = open("/data/btMac", O_RDWR);
    if (fd < 0) {
		printf("open btmac file err!\n");
		return -1;
	}
    else {
        memset(deviceId, '\0', sizeof(deviceId));
        ret=read(fd, deviceId, sizeof(deviceId));
		deviceId[17]='\0';
        printf("ret:%d,get bt mac: %s\n", ret,deviceId);
        close(fd);
    }   
	std::string imsNumber;
	cout << "deviceId:" << deviceId << endl;
	cout << "\n begin call GetIMSAccountNumber!!!!" << endl;
	std::string secretDeviceID = strAESBase64(appSecret, deviceId);
	cout << "secretDeviceID:" << secretDeviceID << endl;

	int account_return = GetIMSAccountNumber(appKey, secretDeviceID, imsNumber);
	cout << "account_return:" << account_return << endl;
	if(account_return == 0){
	    cout << "====================================================" << endl;
	    cout << "本地固话号码：" << imsNumber << endl;
	    cout << "====================================================" << endl;
	}
	else
	{
		cout << "GetIMSAccountNumber has error, with code:" << account_return << endl;
		//cloudphoneRelease(0);
	}
    return account_return;
}
int cloudphoneCall(char* phonenumber, char* contactId,int flag,int* psession) {
    int fd; 
	int ret = 0;
	int session = GetAvailableSessionSeq();
	std::string _phonenumber="";
	string _contactId;
	*psession = session;
	_contactId = contactId;
	_phonenumber = phonenumber;
	if (!strcmp(phonenumber, "(null)"))
		_phonenumber = "";
	if (!strcmp(contactId, "(null)"))
		_contactId = "";

	cout << "_contactId:" << _contactId << endl;
	cout << "_phonenumber:" << _phonenumber << endl;
	if(flag == 20){
		cout << "session:" << session << endl;
		cout << "flag:" << flag << endl;
		ret = CalloutById(session, _contactId, _phonenumber, flag);
		cout << "after CalloutById" << endl;
	}
	else if(flag == 0){
		cout << "session:" << session << endl;
		cout << "flag:" << flag << endl;
		ret = CalloutById(session, _contactId, _phonenumber, flag);
		cout << "after CalloutById" << endl;
	}
	return ret;
}

int cloudphoneConference(int flag) {
    //std::string subject = "test";
	//std::vector<std::string> member;
	//int conferenceType = 0;
	//const std::string sStartTime;
	//std::string pin;
	//int duration;
	//int callType = 0;

	//CreateConference(subject, member, conferenceType, sStartTime, pin, duration, callType);
	
	return 0;
}

int cloudphonPickup(int flag) {
	Pickup(flag);
	return 0;
}

int cloudphoneHangup(int flag) {
	Hangup(flag);
	return 0;
}

int upload_ims_log()
{
	int res = Uploadfile(log_file_name , Agent);

	if(res != 0)
		cout << "Upload failure with code:" << res << endl;
	return res;
}
int cloudphoneRelease(int uploadLogFlag) {
	if(uploadLogFlag)//Upload log to server
    {
#if 0
		string appType = "4";//ims account appType should be "4"
		
		int res = Uploadfile(log_file_name , Agent);
		
		if(res != 0)
			cout << "Upload failure with code:" << res << endl;
#endif
    }
    
	DeregisterUserFromIMSServer();
    DeregisterSignalingObserver();
    DeregisterTraceCallback();
	DestroyEngine();
    cout << "\nexit cloudphone\n" << endl;

	return 0;
}

int cloudphoneReleaseAccount()
{
 
	DeregisterUserFromIMSServer();
    cout << "\nRelease cloudphone account\n" << endl;

	return 0;
}


