/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description 
**  
***************************************************************************/

#ifndef _CLOUDPHONE_H_
#define _CLOUDPHONE_H_

#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_FILE_PATH "/data/migu_voip.log"

typedef void (* cloudphone_OnNetConnected_fun)(void);
typedef void (* cloudphone_OnNetConnetionFailed_fun)(int);
typedef void (* cloudphone_OnUserLoginSucceed_fun)(const char*);
typedef void (* cloudphone_OnUserLoginFailed_fun)(const char*, int, const char*);
typedef void (* cloudphone_OnUserLogout_fun)(const char*);
typedef void (* cloudphone_OnRecvCall_fun)(int, const char*, const char*, const char*, int);
typedef void (* cloudphone_OnRecvAnswer_fun)(int, const char*, const char*, const char*, int);
typedef void (* cloudphone_OnRecvRing_fun)(int, const char*, const char*, const char*, int);
typedef void (* cloudphone_OnResumeSession_fun)(int);
typedef void (* cloudphone_OnRecvHangup_fun)(int, int, const char*);
typedef void (* cloudphone_OnRecvHangupAck_fun)(int, int, const char*);
typedef void (* cloudphone_OnRecvReinvite_fun)(int, const char*, const char*, const char*, int);
typedef void (* cloudphone_OnRecvMessage_fun)(int, const char*, const char*, const char*, const char*);
typedef void (* cloudphone_OnRecvSSRCChanged_fun)(int, int, unsigned int);
typedef void (* cloudphone_OnRecvCSRCChanged_fun)(int, int, unsigned int);
typedef void (* cloudphone_OnCameraStatusChanged_fun)(int, int, int);
typedef void (* cloudphone_OnCallForwarding_fun)(int, const char*, const char*, const char*, int);

typedef void (* cloudphone_OnBFCPSendStop_fun)(int);
typedef void (* cloudphone_OnBFCPSendStart_fun)(int);
typedef void (* cloudphone_OnBFCPRecvStart_fun)(int, int);
typedef void (* cloudphone_OnBFCPRecvStop_fun)(int, int);
typedef void (* cloudphone_OnBFCPMessage_fun)(int, const char*, const char*);

/***************************************************************************************
** FunctionName : cloudphone_OnNetConnected_fun_register
** Description  : 设置连接云固话server端的回调处理函数
** Input Param  : callback
** Output Param : 
** Return Value : 
*****************************************************************************************/
void cloudphone_OnNetConnected_fun_register(cloudphone_OnNetConnected_fun func);

/***************************************************************************************
** FunctionName : cloudphone_OnNetConnetionFailed_fun_register
** Description  : 设置与云固话server端断开的回调处理函数
** Input Param  : callback
** Output Param : 
** Return Value : 
*****************************************************************************************/
void cloudphone_OnNetConnetionFailed_fun_register(cloudphone_OnNetConnetionFailed_fun func);

/***************************************************************************************
** FunctionName : cloudphone_OnUserLoginSucceed_fun_register
** Description  : 设置成功登录云固话系统时的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
*****************************************************************************************/
void cloudphone_OnUserLoginSucceed_fun_register(cloudphone_OnUserLoginSucceed_fun func);

/***************************************************************************************
** FunctionName : cloudphone_OnUserLoginFailed_fun_register
** Description  : 设置失败登录云固话系统时的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
*****************************************************************************************/
void cloudphone_OnUserLoginFailed_fun_register(cloudphone_OnUserLoginFailed_fun func);

/***************************************************************************************
** FunctionName : cloudphone_OnUserLogout_fun_register
** Description  : 设置退出云固话系统时的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
*****************************************************************************************/
void cloudphone_OnUserLogout_fun_register(cloudphone_OnUserLogout_fun func);
void cloudphone_OnRecvCall_fun_register(cloudphone_OnRecvCall_fun func);
void cloudphone_OnRecvAnswer_fun_register(cloudphone_OnRecvAnswer_fun func);
void cloudphone_OnRecvRing_fun_register(cloudphone_OnRecvRing_fun func);
void cloudphone_OnResumeSession_fun_register(cloudphone_OnResumeSession_fun func);
void cloudphone_OnRecvHangup_fun_register(cloudphone_OnRecvHangup_fun func);
void cloudphone_OnRecvHangupAck_fun_register(cloudphone_OnRecvHangupAck_fun func);
void cloudphone_OnRecvReinvite_fun_register(cloudphone_OnRecvReinvite_fun func);
void cloudphone_OnRecvMessage_fun_register(cloudphone_OnRecvMessage_fun func);
void cloudphone_OnRecvSSRCChanged_fun_register(cloudphone_OnRecvSSRCChanged_fun func);
void cloudphone_OnRecvCSRCChanged_fun_register(cloudphone_OnRecvCSRCChanged_fun func);
void cloudphone_OnCameraStatusChanged_fun_register(cloudphone_OnCameraStatusChanged_fun func);
void cloudphone_OnCallForwarding_fun_register(cloudphone_OnCallForwarding_fun func);
void cloudphone_OnBFCPSendStop_fun_register(cloudphone_OnBFCPSendStop_fun func);
void cloudphone_OnBFCPSendStart_fun_register(cloudphone_OnBFCPSendStart_fun func);
void cloudphone_OnBFCPRecvStart_fun_register(cloudphone_OnBFCPRecvStart_fun func);
void cloudphone_OnBFCPRecvStop_fun_register(cloudphone_OnBFCPRecvStop_fun func);
void cloudphone_OnBFCPMessage_fun_register(cloudphone_OnBFCPMessage_fun func);

/************************************************************
** FunctionName : cloudphoneInit
** Description  : 云固话系统登入初始化
** Input Param  : homeserver id：由云固话分配
** Output Param :
** Return Value :
**************************************************************/
int cloudphoneInit(char* sys_version);

int cloudphoneGetAccount(char* homeserver_id);


/************************************************************
** FunctionName : cloudphoneCall
** Description  : 主动拨出电话
** Input Param  : phonenumber：电话号码
**              : contactId: contact id
**              : flag:决定用phonenumber或contact id方式
** Output Param :
** Return Value :
**************************************************************/
int cloudphoneCall(char* phonenumber, char* contactId,int flag,int* session);

int cloudphoneConference(int flag);


/************************************************************
** FunctionName : cloudphonPickup
** Description  : 接听电话
** Input Param  : session：会话id
** Output Param :
** Return Value : 0:成功;others:失败
**************************************************************/
int cloudphonPickup(int flag);


/************************************************************
** FunctionName : cloudphoneHangup
** Description  : 挂断电话
** Input Param  : session：会话id
** Output Param :
** Return Value : 0:成功;others:失败
**************************************************************/
int cloudphoneHangup(int flag);

/************************************************************
** FunctionName : cloudphoneRelease
** Description  : 云固话系统登出
** Input Param  : 
** Output Param :
** Return Value : 0:成功
**************************************************************/
int cloudphoneRelease(int uploadLogFlag);


/************************************************************
** FunctionName : upload_ims_log
** Description  : 云固话系统日记上报
** Input Param  : 
** Output Param :
** Return Value : 0:上报成功,-1:上报失败
**************************************************************/
int upload_ims_log(void);

int cloudphoneReleaseAccount();

#ifdef __cplusplus
}
#endif
#endif


