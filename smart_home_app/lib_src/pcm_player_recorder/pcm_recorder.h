/***************************************************************************
** CopyRight: Amlogic             
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-09
** Description 
**  
***************************************************************************/

#ifndef _PCM_RECORDER_H_
#define _PCM_RECORDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <alsa/asoundlib.h>

/************************************************************
** FunctionName : audiodata_callback
** Description  : 录音数据回调
** Input Param  : 
                   buf: 数据buffer
                   size: 数据长度
                   frames: pcm帧数.( 一般 size = frames * channels * 采样位字节大小)
** Output Param : 
** Return Value : 
**************************************************************/
typedef void (*audiodata_callback)(unsigned char *buf, int size, int frames);


typedef struct _st_pcmrecorder {
	snd_pcm_t *handle;
	char device_name[32];
	int sample_rate_hz;
	int channles;
	snd_pcm_format_t pcm_format;
	int format_bits;
	audiodata_callback callback;
	pthread_t threadid;
	bool running;
}st_pcmrecorder;

typedef st_pcmrecorder *handle_pcmrecorder;



/************************************************************
** FunctionName : pcmrecorder_setcallback
** Description  : 设置录音数据回调
** Input Param  : 
				  handle_pcmrecorder hpcmrecorder 
				  audiodata_callback cb
** Output Param :
** Return Value :
**************************************************************/
void pcmrecorder_setcallback(handle_pcmrecorder hpcmrecorder, audiodata_callback cb);


/************************************************************
** FunctionName : pcmrecorder_init
** Description  : 初始化录音模块
** Input Param  : const char *deviceName 设置录音设备名称 例如 hw:0,2
** Output Param :
** Return Value : handle_pcmrecorder:成功  NULL:失败
**************************************************************/
handle_pcmrecorder pcmrecorder_init(const char *deviceName);


/************************************************************
** FunctionName : pcmrecorder_setchannelcount
** Description  : 设置录音通道数
** Input Param  :
					handle_pcmrecorder hpcmrecorder
					int chnlCnt 录音通道数
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_setchannelcount(handle_pcmrecorder hpcmrecorder, int chnlCnt);


/************************************************************
** FunctionName : pcmrecorder_setformat
** Description  : 设置录音数据格式
** Input Param  :
					handle_pcmrecorder hpcmrecorder
					snd_pcm_format_t format 数据格式
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_setformat(handle_pcmrecorder hpcmrecorder, snd_pcm_format_t format);


/************************************************************
** FunctionName : pcmrecorder_setsamplerate
** Description  : 设置录音采样率(单位Hz)
** Input Param  :
					handle_pcmrecorder hpcmrecorder
					int sample_rate_hz 采样率(单位Hz)
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_setsamplerate(handle_pcmrecorder hpcmrecorder, int sample_rate_hz);

/************************************************************
** FunctionName : pcmrecorder_start
** Description  : 启动录音 (与Stop配对使用，Stop之后可再次Start)
** Input Param  : handle_pcmrecorder hpcmrecorder
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_start(handle_pcmrecorder hpcmrecorder);

/************************************************************
** FunctionName : pcmrecorder_stop
** Description  : 停止录音 (与Start配对使用，Stop之后可再次Start)
** Input Param  : handle_pcmrecorder hpcmrecorder
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_stop(handle_pcmrecorder hpcmrecorder);


/************************************************************
** FunctionName : pcmrecorder_uninit
** Description  : 反初始化
** Input Param  : handle_pcmrecorder hpcmrecorder
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_uninit(handle_pcmrecorder hpcmrecorder);


#ifdef __cplusplus
}
#endif
#endif


