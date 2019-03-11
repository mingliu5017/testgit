#ifndef _AML_PLAYER_MANAGER_H_
#define _AML_PLAYER_MANAGER_H_

#ifdef __cplusplus
extern "C"{ 
#endif

#include "aml_gst_player_imp.h"
#include "pcm_player.h"

typedef enum {
	em_pm_playmode_order = 0,      //顺序播放
	em_pm_playmode_single_cycle,   //单曲循环
	em_pm_playmode_all_cycle,      //全部循环
	em_pm_playmode_random,         //随机
}em_pm_playmode;

typedef enum {
	em_pm_playertype_reserver = 0,
	em_pm_playertype_music    = 1<<0,    //音乐播放器，播放本地文件/xxx/xxx.mp3|wav|..  or http://xxx.xxx/xxx.mp3|wav|..
	em_pm_playertype_notice   = 1<<1,    //提示音播放器，播放本地文件/xxx/xxx.mp3|wav|pcm
	em_pm_playertype_tts      = 1<<2,    //文字转语音播放器
	em_pm_playertype_txtbook  = 1<<3,    //电子书播放器
}em_pm_playertype;


typedef enum{
	em_pm_playerevent_reserver = 0,
	em_pm_playerevent_onefinish,
	em_pm_playerevent_allfinish,
	em_pm_playerevent_playfailed,
}em_pm_playerevent;


typedef void (*playermanager_event_callback)(em_pm_playertype type, em_pm_playerevent e);


/************************************************************
** FunctionName : playermanager_init
** Description  : 播放管理器初始化
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_init();

/************************************************************
** FunctionName : playermanager_set_eventcallback
** Description  : 播放管理器设置回调
** Input Param  : playermanager_event_callback cb
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_set_eventcallback(playermanager_event_callback cb);

/************************************************************
** FunctionName : playermanager_uninit
** Description  : 播放管理器反初始化
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_uninit();


/************************************************************
** FunctionName : playermanager_set_ttsplayer
** Description  : 设置tts播放器
** Input Param  : void *pttsplayer: aml_tts_interface实例内部会转换为aml_tts_interface类指针
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_set_ttsplayer(void *pttsplayer);


/************************************************************
** FunctionName : playermanager_set_playmode
** Description  : 设置播放器模式，仅作用于em_pm_playertype_music
** Input Param  : em_pm_playmode mode
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_set_playmode(em_pm_playmode mode);


/************************************************************
** FunctionName : playermanager_list_add
** Description  : 
                  添加播放资源。
                  对于em_pm_playertype_music,     添加多个将批量播放
                  对于em_pm_playertype_notice,    可以添加播放资源文件的绝对路径
                  对于em_pm_playertype_tts,       content为需要播放的文字
                  对于em_pm_playertype_txtbook, 添加多个将批量播放
** Input Param  : 
					em_pm_playertype type
					const char *content
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_list_add(em_pm_playertype type, const char *content);


/************************************************************
** FunctionName : playermanager_list_clear
** Description  : 清除播放列表，仅对em_pm_playertype_music和em_pm_playertype_txtbook有效
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_list_clear(em_pm_playertype type);


/************************************************************
** FunctionName : playermanager_set_pcmparam
** Description  : 设置播放pcm资源的格式，在playermanager_play之前调用
** Input Param  : 
                  snd_pcm_format_t format PCM格式 
                  int samplerate  采样率
                  int channels    通道数
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_set_pcmparam(snd_pcm_format_t format, int samplerate, int channels);


/************************************************************
** FunctionName : playermanager_play
** Description  : 启动播放。
                  当播放开始，其他播放器会自动暂停或停止。 
                  em_pm_playertype_music、em_pm_playertype_txtbook会暂停，可调用resume恢复播放
                  em_pm_playertype_notice、em_pm_playertype_tts会停止，不可调用resume恢复播放
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_play(em_pm_playertype type);


/************************************************************
** FunctionName : playermanager_pause
** Description  : 暂停播放。
                  如果当前播放器是em_pm_playertype_music、em_pm_playertype_txtbook，会暂停
                  如果当前播放器是em_pm_playertype_notice、em_pm_playertype_tts会停止
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_pause();


/************************************************************
** FunctionName : playermanager_resume
** Description  : 恢复播放。对em_pm_playertype_notice、em_pm_playertype_tts无效
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_resume(em_pm_playertype type);


/************************************************************
** FunctionName : playermanager_stop
** Description  : 停止播放，并清除播放列表
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void playermanager_stop(em_pm_playertype type);

#ifdef __cplusplus
}
#endif
#endif


