/*
 *  Copyright(c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_API_LINUX_VOIP_ENGINE_H_
#define WEBRTC_API_LINUX_VOIP_ENGINE_H_
#include <string>
#include <vector>


enum CALL_TYPE{
    CALL_TYPE_1V1_AUDIO = 0,
    CALL_TYPE_1V1_VIDEO,
    CALL_TYPE_CONFERENCE_AUDIO,
    CALL_TYPE_CONFERENCE_VIDEO,
    CALL_TYPE_BROADCAST_HOST_AUDIO,
    CALL_TYPE_BROADCAST_HOST_VIDEO,
    CALL_TYPE_BROADCAST_ATTE_AUDIO,
    CALL_TYPE_BROADCAST_ATTE_VIDEO,
    CALL_TYPE_LIVE_HOSTER,
    CALL_TYPE_LIVE_VIEWER,
    CALL_TYPE_IMS_1V1_AUDIO = 20,
    CALL_TYPE_IMS_1V1_VIDEO,
    CALL_TYPE_IMS_CONFERENCE_AUDIO,
};
enum NetConnectionFailReason{
    NetConnectionFail_NoError = 0,       //no error
    NetConnectionFail_GetLocalIP,        //failed to get local system ip
    NetConnectionFail_RecvError,         //recv return error
    NetConnectionFail_RecvLenZero,       //server close the link
    NetConnectionFail_NewSocket,         //local error
    NetConnectionFail_Connect,           //server unreachable
    NetConnectionFail_SendError,         //server close the link
    NetConnectionFail_ClientClose,       //client close the socket
    NetConnectionFail_Reconnect,         //client close the socket
};
// EC modes
enum EcModes                   // type of Echo Control
{
    kEcUnchanged = 0,          // previously set mode
    kEcDefault,                // platform default
    kEcConference,             // conferencing default (aggressive AEC)
    kEcAec,                    // Acoustic Echo Cancellation
    kEcAecm,                   // AEC mobile
};
enum AgcModes                  // type of Automatic Gain Control
{
    kAgcUnchanged = 0,        // previously set mode
    kAgcDefault,              // platform default
    // adaptive mode for use when analog volume control exists (e.g. for
    // PC softphone)
    kAgcAdaptiveAnalog,
    // scaling takes place in the digital domain (e.g. for conference servers
    // and embedded devices)
    kAgcAdaptiveDigital,
    // can be used on embedded devices where the capture signal level
    // is predictable
    kAgcFixedDigital
};
enum NsModes    // type of Noise Suppression
{
    kNsUnchanged = 0,   // previously set mode
    kNsDefault,         // platform default
    kNsConference,      // conferencing default
    kNsLowSuppression,  // lowest suppression
    kNsModerateSuppression,
    kNsHighSuppression,
    kNsVeryHighSuppression,     // highest suppression
};

enum ErrorCode
{
	kIvalidappKeyorappSecret = 1001,	//无效的appkey或者appsecret
	kUnrecognizedDeviceModel,			//无法识别的设备型号
	kNotAttatchIMSAccount,				//设备未绑定账号
	kDeviceNotEnabled,					//设备未启用
	kNotOpenHejiaguhua					//尚未开通合家固话
};
class externalTraceCallback{
protected:
    virtual ~externalTraceCallback(){};
public:
    virtual void Print(int level, const char* message, int length)=0;
};


class externalAudioStatisticsObserver{
protected:
    virtual ~externalAudioStatisticsObserver(){};
public:
    virtual void AudioStatistics(int session,
                                 int fractionLost,
                                 int cumulativeLost,
                                 int extendedMax,
                                 int jitterSamples,
                                 int rttMs,
                                 int bytesSent,
                                 int packetsSent,
                                 int bytesReceived,
                                 int packetsReceived)=0;
};
class SignalingObserver{
public:
	SignalingObserver(){};
	virtual ~SignalingObserver(){};
    virtual void OnNetConnected()=0;
    virtual void OnNetConnetionFailed(int errorCode)=0;
    virtual void OnUserLoginSucceed(const char*user)=0;
    virtual void OnUserLoginFailed(const char*user, int errorCode, const char* reason)=0;
    virtual void OnUserLogout(const char*user)=0;
    virtual void OnRecvCall(int session, const char* from, const char* displayname, const char* to, int callType)=0;
    virtual void OnRecvAnswer(int session, const char* from, const char* displayname, const char* to, int callType)=0;
    virtual void OnRecvRing(int session, const char* from, const char* displayname, const char* to, int EarlyMedia)=0;
    virtual void OnResumeSession(int sesion)=0;
    virtual void OnRecvHangup(int session, int errorCode, const char* reason)=0;
    virtual void OnRecvHangupAck(int session, int errorCode, const char* reason)=0;
    virtual void OnRecvReinvite(int session, const char* from, const char* displayname, const char* to, int callType)=0;
    virtual void OnRecvMessage(int session, const char* from, const char* displayname, const char* to, const char* content)=0;
    virtual void OnRecvSSRCChanged(int session, int channel, unsigned int ssrc)=0;
    virtual void OnRecvCSRCChanged(int session, int channel, unsigned int csrc)=0;
    virtual void OnCameraStatusChanged(int session, int id, int status)=0;//status:0/others  0:success, -1:startcapture failed, -2:allocate camera failed,-3, no camera
    virtual void OnCallForwarding(int session, const char* from, const char* displayname, const char* to, int callType)=0;  
	virtual void OnBFCPSendStart(int session) = 0;
	virtual void OnBFCPSendStop(int session) = 0;
	virtual void OnBFCPRecvStart(int session, int channleID) = 0;
	virtual void OnBFCPRecvStop(int session, int channleID) = 0;
	virtual void OnBFCPMessage(int session, const char* obj, const char* action) = 0;
};
#ifdef __cplusplus
extern "C" {
#endif

int RegisterUserToServer(std::string sSerAddr,
			 int iPort,
			 std::string sUserName,
			 std::string sPassWord);
/**
RegisterUserToOTTServer() 用户注册
@param sSerAddr     服务器IP地址
@param iPort        服务器端口
@param sUserName    用户名
@param sAppkey      AppKey
@param sToken       密码
@return 返回值，0-成功，其他-失败
**/
int RegisterUserToOTTServer(std::string sSerAddr,
			 int iPort,
			 std::string sUserName,
			 std::string sAppkey,
			 std::string sToken);
int RegisterUser2IMSServer(
				 std::string serverlUrl,
	             std::string serverAddr,
			     int iPort,
			     std::string userName,
			     std::string displayName,
			     std::string authName,
			     std::string password);
/**
GetUserPasswordEncrypted() 获取加密后的用户密码
@return 加密后的密码字符串
**/
std::string GetUserPasswordEncrypted();
/**
DeregisterUserFromOTTServer() 从服务器注销当前OTT账户
@return 返回值，0-成功，其他-失败
**/
int DeregisterUserFromOTTServer();
/**
DeregisterUserFromIMSServer() 从服务器注销当前IMS账户
@return 返回值，0-成功，其他-失败
**/
int DeregisterUserFromIMSServer();

/**
RegisterTraceCallback() 注册TraceCallback
@param callback TraceCallback指针
@param logPath, 需要保存log信息时传入log文件地址，无需保存log信息时传入空字符串 "" 即可
@return 返回值，0-成功，其他-失败
**/
int RegisterTraceCallback(externalTraceCallback* callback, std::string logPath);
int DeregisterTraceCallback();
int SetTraceFilter(int filter);
int RegisterSignalingObserver(SignalingObserver* observer);
int DeregisterSignalingObserver();
int RegisterAudioStatisticsObserver(externalAudioStatisticsObserver *observer) ;
int DeregisterAudioStatisticsObserver();
int GetAvailableSessionSeq();
int SetCallForwardingFlag(int enable);

/**
GetIMSAccountNumber() 获取ims账号
@param appKey 输入参数，应用的key

@param deviceId 输入参数，设备ID
@param imsNumber 输出参数，返回固话号码
@return 返回值，0-成功，其他-失败
**/

int GetIMSAccountNumber(std::string appKey, std::string deviceId, std::string &imsNumber);


/**
CreateEngine() 创建引擎
@return 返回值，0-成功，其他-失败
**/
int CreateEngine();
/**
DestroyEngine() 销毁引擎
@return 返回值，0-成功，其他-失败
**/
void DestroyEngine();
/**
Callout() 发起呼叫
@param session 会话id
@param strNumber 被叫号码
@param type 呼叫类型 0-1v1音频 1-1v1视频 20-1v1ims音频 21-1v1ims视频
@return 返回值，0-成功，其他-失败
**/
int Callout(int session, std::string strNumber, int type);
/**
CalloutById() 发起电话呼叫，当phoneNumber存在时以phoneNumber呼叫；否则以通信录ID发起呼叫
@param session 会话id
@param contactId 通信录ID
@param  phoneNumber 被叫号码
@param type 呼叫类型 0-1v1音频 1-1v1视频 20-1v1ims音频 21-1v1ims视频
@return 返回值，0-成功，其他-失败
**/
int CalloutById(int session, std::string contactId, std::string phoneNumber, int callType);

/**
Pickup() 接听
@param session 会话id
@return 返回值，0-成功，其他-失败
**/
int Pickup(int session);
/**
Hangup() 挂断
@param session 会话id
@return 返回值，0-成功，其他-失败
**/
int Hangup(int session);
/**
SendDtmf() 发送DTMF
@param session  会话id
@param code     dtmf code
@return 返回值，0-成功，其他-失败
**/
int SendDtmf(int session,int code);
/**
SetSpeakerVolume() 设置扬声器音量
@param session  会话id
@param level    扬声器音量
@return 返回值，0-成功，其他-失败
**/
int SetSpeakerVolume(int session, int level);
/**
GetSpeakerVolume() 获取扬声器音量
@param session 会话id
@return 返回值，扬声器音量
**/
int GetSpeakerVolume(int session);
/**
SetOutputMute() 设置输出为静音
@param session  会话id
@param enable   0-不静音，1-静音
@return 返回值，0-成功，其他-失败
**/
int SetOutputMute(int session, bool enable);
/**
GetOutputMute() 获取输出静音状态
@param session 会话id
@return 返回值，false-不静音，true-静音
**/
bool GetOutputMute(int session) ;
/**
SetInputMute() 设置输入静音
@param session  会话id
@param enable   false-不静音，true-静音
@return 返回值，0-成功，其他-失败
**/
int SetInputMute(int session, bool enable);
/**
GetInputMute() 获取输入的静音状态
@param session 会话id
@return 返回值，false-不静音，true-静音
**/
bool GetInputMute(int session);
/**
SetEcStatus() 回声消除开关
@param enable为True表示打开，为False表示关闭
@param ec_mode AEC模式
@return 返回值，0表示成功，非0表示失败
**/

int SetEcStatus(bool enable,int ec_mode);
/**
SetAgcStatus() 增益控制开关
@param enable为True表示打开，为False表示关闭
@param agc_mode 增益控制模式
@return 返回值，0表示成功，非0表示失败
**/

int SetAgcStatus(bool enable, int agc_mode);
/**
SetNsStatus() 降噪开关
@param enable为True表示打开，为False表示关闭
@param ns_mode 降噪模式
@return 返回值，0表示成功，非0表示失败
**/
int SetNsStatus(bool enable,int ns_mode);
/**
CreateConference() 创建一个会议
@param subject 会议主题
@param member 成员列表
@param conferenceType 会议类型
@param sStartTime 开始时间
@param pin
@param duration 会议持续时间
@param callType 呼叫类型
@return 返回值，0表示成功，非0表示失败
**/
int CreateConference(const std::string subject,
                        const std::vector<std::string> member,
                        int conferenceType,
                        const std::string sStartTime,
                        const std::string pin,
                        int duration,
                        int callType);
/**
CloseConference() 关闭会议
@param iSession 会议ID
@return 返回值，0表示成功，非0表示失败
**/

int CloseConference(int iSession);
/**
JoinConference() 加入一个会议
@param iSession 会议ID
@param number 成员号码
@param callType 呼叫类型
@return 返回值，0表示成功，非0表示失败
**/
int JoinConference(int iSession, std::string number, int callType);
/**
InviteConferenceMembers() 邀请会议成员
@param iSession 会议ID
@param member成员列表
@return 返回值，0表示成功，非0表示失败
**/

int InviteConferenceMembers(int iSession, std::vector<std::string> member);
/**
KickOutConferenceMember() 将一个或多个成员移出会议
@param iSession 会议ID
@param member 成员列表
@return 返回值，0表示成功，非0表示失败
**/
int KickOutConferenceMember(int iSession, std::vector<std::string> member);
int RequestConferenceInfo(int iSession);
/**
LockConference() 锁定一个会议 
@param iSession 会议ID
@param lock为True表示锁定，lock为False表示解除锁定
@return 返回值，0表示成功，非0表示失败
**/
int LockConference(int iSession, bool lock);
/**
SetConferenceMemberMute() 使某个会议成员静音
@param iSession 会议ID
@param memberUser 会议成员
@param mute True表示使之静音，False表示取消静音
@return 返回值，0表示成功，非0表示失败
**/
int SetConferenceMemberMute(int iSession, const std::string memberUser, bool mute);
/**
GetVersion() 获取SDK版本号
@return 返回值，版本号字符串
**/
std::string GetSDKVersion();

/**
Uploadfile() 上传文件到服务器，一般用于异常发生时上传log文件
@param filepath, 文件的本地路径
@param Agent, 用于表示UserAgent

@return 返回值，0表示成功，非0表示失败
**/
int Uploadfile(std::string filepath, 
				std::string Agent);

/**
PhoneNumber2Name() 获取通信录里输入手机号的名字
@param phoneNumber, 电话号码
@param phoneName, 用于返回名字
@return 返回值，0表示成功，非0表示失败
**/
int PhoneNumber2Name(std::string phoneNumber, 
						std::string &phoneName);

/**
 * strAESBase64() 对deviceId做AES加密，然后base64编码
 * @param strKey, 加密的Key，传入appSecret作为Key
 * @param deviceId, 被加密的字符串，传入deviceId
 * @return 返回值，返回加密并编码后的字符串
 * **/
std::string strAESBase64(std::string strKey, std::string deviceId);

#ifdef __cplusplus
}
#endif


#endif  // WEBRTC_API_LINUX_VOIP_ENGINE_H_
