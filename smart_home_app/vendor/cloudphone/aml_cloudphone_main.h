/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description 
**  
***************************************************************************/

#ifndef _AML_CLOUDPHONE_MAIN_H_
#define _AML_CLOUDPHONE_MAIN_H_

#ifdef __cplusplus
extern "C"{ 
#endif

#include "statusevent_manage.h"

/************************************************************
** FunctionName : callingEventHandle
** Description  : 云固话事件处理
** Input Param  : pPhoneNode
** Output Param : 
** Return Value : 
**************************************************************/
void callingEventHandle(phoneNode_t* pPhoneNode);

/************************************************************
** FunctionName : aml_enterCloudphone
** Description  : 登录云固话系统
** Input Param  : void
** Output Param : 
** Return Value : 
**************************************************************/
int aml_enterCloudphone(void);

enum PHONE_CMD {
	PHONE_INIT = 0,
	PHONE_BIND,
	PHONE_UNBIND,
	PHONE_CALLOUT,
	PHONE_DEREGIST,
	PHONE_IDLE,
};


#ifdef __cplusplus
}
#endif
#endif


