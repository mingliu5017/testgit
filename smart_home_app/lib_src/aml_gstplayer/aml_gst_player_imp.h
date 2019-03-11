/***************************************************************************
** CopyRight: Amlogic
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-28
** Description
**
***************************************************************************/

#ifndef _AML_GSTPLAYER_
#define _AML_GSTPLAYER_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _em_gst_play_status {
	EM_GSTPLAYER_STATUS_UNKNOWN,
	EM_GSTPLAYER_STATUS_INIT,
	EM_GSTPLAYER_STATUS_PLAYING,
	EM_GSTPLAYER_STATUS_PAUSE,
	EM_GSTPLAYER_STATUS_STOPPED,
} aml_gstplayer_status;

typedef enum _em_gst_play_event {
	EM_GSTPLAYER_EVENT_PlayFinished,
	EM_GSTPLAYER_EVENT_PlayError
} aml_gstplayer_event;


typedef struct _st_gstplayer st_gstplayer;

/************************************************************
** FunctionName : aml_gstplayer_event_cb
** Description  : 播放事件回调
** Input Param  : 
┌───────────────────────────────────┬────────────────────┐
│    event                          │    param           │
├───────────────────────────────────┼────────────────────┤
│  EM_GSTPLAYER_EVENT_PlayFinished  │    NULL            │
├───────────────────────────────────┼────────────────────┤
│  EM_GSTPLAYER_EVENT_PlayError     │    NULL            │
└───────────────────────────────────┴────────────────────┘
** Output Param :
** Return Value :
**************************************************************/
typedef void (*aml_gstplayer_event_cb)(st_gstplayer *pgstplayer, aml_gstplayer_event event, void *param);


/************************************************************
** FunctionName : aml_gstplayer_onprogress
** Description  : 播放进度回调
** Input Param  :
** Output Param :
				unsigned int current 当前播放时间点，单位ms
				unsigned int duration 总时长，单位ms
** Return Value :
**************************************************************/
typedef void (*aml_gstplayer_onprogress)(st_gstplayer *pgstplayer, unsigned int current, unsigned int duration);


/************************************************************
** FunctionName : aml_gstplayer_init
** Description  : 初始化，设置事件回调和播放进度回调
** Input Param  :
					aml_gstplayer_event_cb cb 播放事件回调
					aml_gstplayer_onprogress OnProgress 播放进度回调
** Output Param :
** Return Value :  hamlgstplayer:success   NULL:failed
**************************************************************/
st_gstplayer *aml_gstplayer_init(aml_gstplayer_event_cb cb, aml_gstplayer_onprogress OnProgress);


/************************************************************
** FunctionName : aml_gstplayer_play
** Description  : 播放url  ，例如 file:///data/test.mp3    ,  http://xxx.xxx.xxx/test.mp3
** Input Param  :
					const st_gstplayer *pgstplayer
					const char *pUrl
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_play(st_gstplayer *pgstplayer, const char *pUrl);


/************************************************************
** FunctionName : aml_gstplayer_pause
** Description  : 暂停
** Input Param  : hamlgstplayer hgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_pause(st_gstplayer *pgstplayer);


/************************************************************
** FunctionName : aml_gstplayer_resume
** Description  : 暂停恢复
** Input Param  : hamlgstplayer hgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_resume(st_gstplayer *pgstplayer);


/************************************************************
** FunctionName : aml_gstplayer_stop
** Description  : 停止
** Input Param  : hamlgstplayer hgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_stop(st_gstplayer *pgstplayer);


/************************************************************
** FunctionName : aml_gstplayer_seek
** Description  : 从指定位置开始播放
** Input Param  : 
					hamlgstplayer hgstplayer
					int pos，单位ms
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_seek(st_gstplayer *pgstplayer, int pos);


/************************************************************
** FunctionName : aml_gstplayer_get_duration
** Description  : 获取时长，单位ms
** Input Param  : hamlgstplayer hgstplayer
** Output Param :
** Return Value : 时长单位ms， 失败返回0
**************************************************************/
int aml_gstplayer_get_duration(st_gstplayer *pgstplayer);


/************************************************************
** FunctionName : aml_gstplayer_get_status
** Description  : 获取状态
** Input Param  : hamlgstplayer hgstplayer
** Output Param :
** Return Value : aml_gstplayer_status
**************************************************************/
aml_gstplayer_status aml_gstplayer_get_status(st_gstplayer *pgstplayer);


/************************************************************
** FunctionName : aml_gstplayer_uninit
** Description  : 反初始化
** Input Param  : st_gstplayer **pgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_uninit(st_gstplayer *pgstplayer);


#ifdef __cplusplus
}
#endif
#endif


