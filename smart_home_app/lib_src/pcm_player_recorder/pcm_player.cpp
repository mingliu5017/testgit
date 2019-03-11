/***************************************************************************
** CopyRight: Amlogic
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-09
** Description
**
***************************************************************************/

#define LOG_TAG "pcm_player"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "aml_log.h"
#include "util.h"
#include "pcm_player.h"

//static snd_pcm_t *handle = NULL;
//static char play_device_name[32] = {"dmixer_auto"};
//static int s_sample_rate_hz = 16000;
//static int s_channles = 2;
//static snd_pcm_format_t s_pcm_format = SND_PCM_FORMAT_S16_LE;
//static int s_format_bits = 16;

//static bool s_ready = false;

/************************************************************
** FunctionName : pcmplayer_init
** Description  : 初始化播放模块
** Input Param  : 
					const char *deviceName 设置播放设备名称 例如 hw:0,2 或者 dmixer_auto
** Output Param :
** Return Value : handle_pcmplayer:成功  NULL:失败
**************************************************************/
handle_pcmplayer pcmplayer_init(const char *deviceName)
{
	LOG(LEVEL_INFO, "deviceName:%s", deviceName);

	handle_pcmplayer hpcmplayer= (handle_pcmplayer)malloc(sizeof(st_pcmplayer));
	memset(hpcmplayer, 0, sizeof(st_pcmplayer));
	
	int size = sizeof(hpcmplayer->device_name);
	snprintf(hpcmplayer->device_name, size - 1, "%s", deviceName);

	hpcmplayer->channles = 2;
	hpcmplayer->sample_rate_hz = 16000;
	hpcmplayer->pcm_format = SND_PCM_FORMAT_S16_LE;
	hpcmplayer->format_bits = 16;

	/* Open PCM device for play. */
	/* 打开 PCM capture 捕获设备 */
	int rc = snd_pcm_open(&hpcmplayer->handle, deviceName, SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		LOG(LEVEL_FATAL, "unable to open pcm device(%s): %s", deviceName, snd_strerror(rc));
		free(hpcmplayer);
		return NULL;
	}

	return hpcmplayer;
}


/************************************************************
** FunctionName : pcmplayer_setchannelcount
** Description  : 设置播放通道数
** Input Param  : 
				  handle_pcmplayer handle,
				  int chnlCnt
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_setchannelcount(handle_pcmplayer hpcmplayer, int chnlCnt)
{
	hpcmplayer->channles = chnlCnt;
	return 0;
}


/************************************************************
** FunctionName : pcmplayer_setformat
** Description  : 设置播放数据格式
** Input Param  :
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_setformat(handle_pcmplayer hpcmplayer, snd_pcm_format_t format)
{
	hpcmplayer->pcm_format = format;
	switch(hpcmplayer->pcm_format) {
		case SND_PCM_FORMAT_S8:
		case SND_PCM_FORMAT_U8:
			hpcmplayer->format_bits = 8;
			break;
		case SND_PCM_FORMAT_S16_LE:
		case SND_PCM_FORMAT_S16_BE:
		case SND_PCM_FORMAT_U16_LE:
		case SND_PCM_FORMAT_U16_BE:
			hpcmplayer->format_bits = 16;
			break;
		case SND_PCM_FORMAT_S24_LE:
		case SND_PCM_FORMAT_S24_BE:
		case SND_PCM_FORMAT_U24_LE:
		case SND_PCM_FORMAT_U24_BE:
			hpcmplayer->format_bits = 24;
			break;
		case SND_PCM_FORMAT_S32_LE:
		case SND_PCM_FORMAT_S32_BE:
		case SND_PCM_FORMAT_U32_LE:
		case SND_PCM_FORMAT_U32_BE:
			hpcmplayer->format_bits = 32;
			break;
		default:
			return -1;
	}

	return 0;
}


/************************************************************
** FunctionName : pcmplayer_setsamplerate
** Description  : 设置播放采样率(单位Hz)
** Input Param  :
					handle_pcmplayer hpcmplayer
					int sample_rate_hz 采样率(单位Hz)
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_setsamplerate(handle_pcmplayer hpcmplayer, int sample_rate_hz)
{
	hpcmplayer->sample_rate_hz = sample_rate_hz;
	return 0;
}

/************************************************************
** FunctionName : pcmplayer_start
** Description  : 启动播放 (与Stop配对使用，Stop之后可再次Start)
** Input Param  :
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_start(handle_pcmplayer hpcmplayer)
{
	LOG(LEVEL_INFO, "");

	int rc = 0;

	snd_pcm_hw_params_t *params;
	unsigned int val = 0;
	int dir;

	if(hpcmplayer->handle == NULL) {
		LOG(LEVEL_ERROR, "hpcmplayer->handle is NULL!");
		return -1;
	}

	/* Allocate a hardware parameters object. */
	/* 分配一个硬件参数结构体 */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	/* 使用默认参数 */
	snd_pcm_hw_params_any(hpcmplayer->handle, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	snd_pcm_hw_params_set_access(hpcmplayer->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	/* 格式 */
	snd_pcm_hw_params_set_format(hpcmplayer->handle, params, hpcmplayer->pcm_format);

	/* 通道 */
	snd_pcm_hw_params_set_channels(hpcmplayer->handle, params, hpcmplayer->channles);

	/* 采样率 */
	val = hpcmplayer->sample_rate_hz;
	snd_pcm_hw_params_set_rate_near(hpcmplayer->handle, params, &val, &dir);

#if 0
	/* Set period size to 32 frames. */
	snd_pcm_uframes_t frames;
	frames = 4096;
	snd_pcm_hw_params_set_period_size_near(hpcmplayer->handle, params, &frames, &dir);
#else
	{
		unsigned buffer_time = 0;
		unsigned period_time = 0;
		snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
		if(buffer_time > 500 * 1000) {
			buffer_time = 500 * 1000;
		}

		period_time = buffer_time / 4;
		snd_pcm_hw_params_set_period_time_near(hpcmplayer->handle, params, &period_time, 0);
		snd_pcm_hw_params_set_buffer_time(hpcmplayer->handle, params, buffer_time, 0);

	}
#endif
	/* Write the parameters to the driver */
	/* 参数生效 */
	rc = snd_pcm_hw_params(hpcmplayer->handle, params);
	if (rc < 0) {
		LOG(LEVEL_INFO, "unable to set hw parameters: %s", snd_strerror(rc));
		//fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		exit(1);
	}

	LOG(LEVEL_INFO, "s_pcm_format:%d channels:%d sample_rate:%d", hpcmplayer->pcm_format, hpcmplayer->channles, 
		hpcmplayer->sample_rate_hz);

	snd_pcm_prepare(hpcmplayer->handle);
#if 0
	{
		snd_output_t *log;
		snd_output_stdio_attach(&log, stderr, 0);
		snd_pcm_dump(hpcmplayer->handle, log);
		snd_output_close(log);
	}
#endif
	return 0;
}

/************************************************************
** FunctionName : pcmplayer_writedata
** Description  : 向播放设备写入数据
** Input Param  :
					const unsigned char *buf:  数据buffer
					int size：数据长度
					int frames：数据包含的pcm帧数
** Output Param :
** Return Value : -1:失败  0:写入pcm frame数的长度
**************************************************************/
int pcmplayer_writedata(handle_pcmplayer hpcmplayer, const unsigned char *buf, int size, int frames)
{
	int ret = 0;

	if(hpcmplayer->handle == NULL || buf == NULL || frames <=0) {
		LOG(LEVEL_ERROR, "hpcmplayer->handle:%p buf:%p frames:%d", hpcmplayer->handle, buf, frames);
		return -1;
	}

	ret = snd_pcm_writei(hpcmplayer->handle, buf, frames);

	if (ret == -EPIPE) {
		/* EPIPE means underrun */
		//fprintf(stderr, "underrun occurred\n");
		LOG(LEVEL_ERROR, "underrun occurred");
		snd_pcm_prepare(hpcmplayer->handle);
		ret = 0;
	} else if (ret < 0) {
		//fprintf(stderr, "error from writei: %s\n",snd_strerror(ret));
		LOG(LEVEL_ERROR, "error from writei: %s", snd_strerror(ret));
		return -1;
	}  else if (ret != frames) {
		//fprintf(stderr, "short write, write %d frames\n", ret);
		LOG(LEVEL_ERROR, "short write, write %d frames", ret);
	}
	
	return ret;
}

/************************************************************
** FunctionName : pcmplayer_uninit
** Description  : 反初始化
** Input Param  :
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmplayer_uninit(handle_pcmplayer hpcmplayer)
{
	LOG(LEVEL_INFO, "");

	if(hpcmplayer->handle != NULL) {
		snd_pcm_drop(hpcmplayer->handle);
		snd_pcm_close(hpcmplayer->handle);
		hpcmplayer->handle = NULL;
	}

	free(hpcmplayer);

	return 0;
}


