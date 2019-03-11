/***************************************************************************
** CopyRight: Amlogic             
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-09
** Description 
**  
***************************************************************************/

#ifndef __PCMPLAYER_H_
#define __PCMPLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <alsa/asoundlib.h>

typedef struct _st_pcmplayer {
	snd_pcm_t *handle;
	char device_name[32];
	int sample_rate_hz;
	int channles;
	snd_pcm_format_t pcm_format;
	int format_bits;
}st_pcmplayer;

typedef st_pcmplayer *handle_pcmplayer;

/************************************************************
** FunctionName : pcmplayer_init
** Description  : 初始化播放模块
** Input Param  : 
					const char *deviceName 设置播放设备名称 例如 hw:0,2
** Output Param :
** Return Value : handle_pcmplayer:成功  NULL:失败
**************************************************************/
handle_pcmplayer pcmplayer_init(const char *deviceName);


/************************************************************
** FunctionName : pcmplayer_setchannelcount
** Description  : 设置播放通道数
** Input Param  : 
					handle_pcmplayer handle
					int chnlCnt 播放通道数
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_setchannelcount(handle_pcmplayer hpcmplayer, int chnlCnt);


/************************************************************
** FunctionName : pcmplayer_setformat
** Description  : 设置播放数据格式
** Input Param  : 
					handle_pcmplayer handle
					snd_pcm_format_t format 播放数据格式
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_setformat(handle_pcmplayer hpcmplayer, snd_pcm_format_t format);


/************************************************************
** FunctionName : pcmplayer_setsamplerate
** Description  : 设置播放采样率(单位Hz)
** Input Param  : 
					handle_pcmplayer handle
					int sample_rate_hz 播放采样率(单位Hz)
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_setsamplerate(handle_pcmplayer hpcmplayer, int sample_rate_hz);

/************************************************************
** FunctionName : pcmplayer_start
** Description  : 启动播放
** Input Param  : 
					handle_pcmplayer handle
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_start(handle_pcmplayer hpcmplayer);

/************************************************************
** FunctionName : pcmplayer_writedata
** Description  : 向播放设备写入数据
** Input Param  :
					handle_pcmplayer handle
					const unsigned char *buf:  数据buffer
					int size：数据长度
					int frames：数据包含的pcm帧数
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_writedata(handle_pcmplayer hpcmplayer, const unsigned char *buf, int size, int frames);


/************************************************************
** FunctionName : pcmplayer_uninit
** Description  : 反初始化
** Input Param  : handle_pcmplayer handle
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_uninit(handle_pcmplayer hpcmplayer);


#ifdef __cplusplus
}
#endif
#endif


