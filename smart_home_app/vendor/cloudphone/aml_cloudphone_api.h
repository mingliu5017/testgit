/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description 
**  
***************************************************************************/

#ifndef _AML_CLOUDPHONE_API_H_
#define _AML_CLOUDPHONE_API_H_

#ifdef __cplusplus
extern "C"{ 
#endif

#include "cloudphone.h"

/************************************************************
** FunctionName : cloudphone_onRecvCall_setCallback
** Description  : 设置接收到某人来电的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onRecvCall_setCallback(cloudphone_OnRecvCall_fun callback);

/************************************************************
** FunctionName : cloudphone_onRecvRing_setCallback
** Description  : 设置对方铃响的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onRecvRing_setCallback(cloudphone_OnRecvRing_fun callback);

/************************************************************
** FunctionName : cloudphone_onRecvRing_setCallback
** Description  : 设置被对方挂断的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onRecvHangup_setCallback(cloudphone_OnRecvHangup_fun callback);

/************************************************************
** FunctionName : cloudphone_onRecvHangupAck_setCallback
** Description  : 设置被电话挂断的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onRecvHangupAck_setCallback(cloudphone_OnRecvHangupAck_fun callback);

/************************************************************
** FunctionName : cloudphone_onRecvAnswer_setCallback
** Description  : 设置对方接听电话时的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onRecvAnswer_setCallback(cloudphone_OnRecvAnswer_fun callback);

/************************************************************
** FunctionName : cloudphone_onLoginSucced_setCallback
** Description  : 设置成功登录云固话系统时的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onLoginSucced_setCallback(cloudphone_OnUserLoginSucceed_fun callback);

/************************************************************
** FunctionName : cloudphone_onLoginFailed_setCallback
** Description  : 设置失败登录云固话系统时的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onLoginFailed_setCallback(cloudphone_OnUserLoginFailed_fun callback);

/************************************************************
** FunctionName : cloudphone_onLoginOut_setCallback
** Description  : 设置退出云固话系统时的事件处理
** Input Param  : callback
** Output Param : 
** Return Value : 
**************************************************************/
void cloudphone_onLoginOut_setCallback(cloudphone_OnUserLogout_fun callback);

int cloudphone_hangup();


#ifdef __cplusplus
}
#endif

#endif


