#ifndef __HFP_CTL_H
#define __HFP_CTL_H
#ifdef __cplusplus
extern "C" {
#endif

#define HFP_EVENT_CONNECTION 1
#define HFP_EVENT_CALL       2
#define HFP_EVENT_CALLSETUP  3
#define HFP_EVENT_VGS        4
#define HFP_EVENT_VGM        5

#define HFP_IND_DEVICE_DISCONNECTED 0
#define HFP_IND_DEVICE_CONNECTED    1

#define HFP_IND_CALL_NONE           0
/* at least one call is in progress */
#define HFP_IND_CALL_ACTIVE         1
/* currently not in call set up */
#define HFP_IND_CALLSETUP_NONE      0
/* an incoming call process ongoing */
#define HFP_IND_CALLSETUP_IN        1
/* an outgoing call set up is ongoing */
#define HFP_IND_CALLSETUP_OUT       2
/* remote party being alerted in an outgoing call */
#define HFP_IND_CALLSETUP_OUT_ALERT 3


/************************************************************
** typeName : hfp_event_cb
** Description  : hfp 事件回调函数类型
** Input Param  : event：事件类型; value:事件携带的参数。event和对应的value如下
event:HFP_EVENT_CONNECTION	(设备连接状态变化事件)
	value:	HFP_IND_DEVICE_DISCONNECTED	(设备断开)
			HFP_IND_DEVICE_CONNECTED	(设备接入)
event:HFP_EVENT_CALL	(通话事件)
	value:	HFP_IND_CALL_NONE	(结束通话状态)
		  	HFP_IND_CALL_ACTIVE (进入通话状态)
event:HFP_EVENT_CALLSETUP	(呼叫事件)
	value:	HFP_IND_CALLSETUP_NONE	(呼叫挂断)
			HFP_IND_CALLSETUP_IN	(电话呼入)
			HFP_IND_CALLSETUP_OUT	(电话呼出)
			HFP_IND_CALLSETUP_OUT_ALERT	(对方收到呼叫开始振铃)
event:HFP_EVENT_VGS	(speaker音量变化)
	value 0-15	(当前音量值)
event:HFP_EVENT_VGM	(mic 音量变化)
	value 0-15	(当前音量值)

** Output Param :
** Return Value :
**************************************************************/
typedef void (*hfp_event_cb)(int event, int value);
/************************************************************
** FunctionName : hfp_set_event_cb
** Description  : 设置事件回调函数
** Input Param  : callback用户回调函数
** Output Param :
** Return Value :
**************************************************************/
void hfp_set_event_cb(hfp_event_cb callback);

/************************************************************
** FunctionName : hfp_ctl_init
** Description  : 初始化函数，调用本函数并返回成功后，才可以调用其他函数，只有hfp_set_event_cb在此函数之前调用。
** Input Param  :
** Output Param :
** Return Value : 0 successful; -1 failed
**************************************************************/
int hfp_ctl_init(void);
/************************************************************
** FunctionName : hfp_ctl_delinit
** Description  : 反初始化函数
** Input Param  :
** Output Param :
** Return Value :
**************************************************************/
void hfp_ctl_delinit(void);
/************************************************************
** FunctionName : hfp_answer_call
** Description  : 对方来电时，调用此函数接听
** Input Param  :
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_answer_call(void);
/************************************************************
** FunctionName : hfp_reject_call
** Description  : 对方来电时或者通话状态时调用此函数函数挂断
** Input Param  :
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_reject_call(void);
/************************************************************
** FunctionName : hfp_VGS_up
** Description  : speaker 音量减
** Input Param  :
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_VGS_up(void);
/************************************************************
** FunctionName : hfp_VGS_down
** Description  : speaker 音量加
** Input Param  :
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_VGS_down(void);
/************************************************************
** FunctionName : hfp_VGM_up
** Description  : mic音量加
** Input Param  :
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_VGM_up(void);
/************************************************************
** FunctionName : hfp_VGM_down
** Description  : mic音量减
** Input Param  :
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_VGM_down(void);
/************************************************************
** FunctionName : hfp_set_VGS
** Description  : 设置speaker音量
** Input Param  : 音量值，有效值0-15
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_set_VGS(int value);
/************************************************************
** FunctionName : hfp_set_VGM
** Description  : 设置mic音量
** Input Param  : 音量值，有效值0-15
** Output Param :
** Return Value : 0 successfull; 其他值 failed
**************************************************************/
int hfp_set_VGM(int value);

/************************************************************
** FunctionName : hfp_is_connected
** Description  : 查询当前连接状态
** Input Param  :
** Output Param :
** Return Value :  0 未连接状态; 1 连接状态
**************************************************************/
int hfp_is_connected(void);

#ifdef __cplusplus
}
#endif
#endif

