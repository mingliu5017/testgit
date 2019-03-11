#ifndef _A2DP_H_
#define _A2DP_H_
#ifdef __cplusplus
extern "C"{ 
#endif

#include <glib.h>
/*param possible value: FALSE, TRUE*/
typedef void (*a2dp_connect_status_cb)(gboolean connected);
/*PARAM Possible value: "playing", "stopped", "paused"*/
typedef void (*a2dp_play_status_cb)(char *status);

/************************************************************
** FunctionName : a2dp_player_init
** Description  : 初始化函数，在调用本文件中其他函数之前需要调用本函数,除了
				  a2dp_set_connect_status_callback和a2dp_set_play_status_callback
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_player_init(void);
/************************************************************
** FunctionName : a2dp_player_delinit
** Description  : 反初始化函数，释放资源
** Input Param  :
** Output Param :
** Return Value :
**************************************************************/
void a2dp_player_delinit(void);
/************************************************************
** FunctionName : a2dp_start
** Description  : 发送play命令给source端，开始播放
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_start(void);
/************************************************************
** FunctionName : a2dp_stop
** Description  : 发送stop命令给source端，
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_stop(void);
/************************************************************
** FunctionName : a2dp_pause_play
** Description  : pause
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_pause(void);
/************************************************************
** FunctionName : a2dp_next
** Description  : next
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_next(void);
/************************************************************
** FunctionName : a2dp_previous
** Description  : previous
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_previous(void);
/************************************************************
** FunctionName : a2dp_volume_up
** Description  : volume up
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_volume_up(void);
/************************************************************
** FunctionName : a2dp_volume_down
** Description  : volume down
** Input Param  :
** Output Param :
** Return Value : 0: successful;	-1:failed
**************************************************************/
int a2dp_volume_down(void);
/************************************************************
** FunctionName : a2dp_is_connected
** Description  : 查询是否有支持a2dp的设备连接
** Input Param  :
** Output Param :
** Return Value : TRUE: 连接状态;	FALSE:未连接状态
**************************************************************/
gboolean a2dp_is_connected(void);
/************************************************************
** FunctionName : a2dp_set_connect_status_callback
** Description  : 注册连接状态变化的回调函数，连接状态变化时回调
** Input Param  : callback：回调函数。
** Output Param :
** Return Value :
**************************************************************/
void a2dp_set_connect_status_callback(a2dp_connect_status_cb callback);
/************************************************************
** FunctionName : a2dp_set_play_status_callback
** Description  : 注册播放事件的回调函数，播放状态变化时回调
** Input Param  : callback：回调函数
** Output Param :
** Return Value :
**************************************************************/
void a2dp_set_play_status_callback(a2dp_play_status_cb callback);

#ifdef __cplusplus
}
#endif
#endif
