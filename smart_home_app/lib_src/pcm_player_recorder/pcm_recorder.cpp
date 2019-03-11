/***************************************************************************
** CopyRight: Amlogic
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-09
** Description
**
***************************************************************************/

#define LOG_TAG "pcm_recorder"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "aml_log.h"
#include "util.h"

#include "pcm_recorder.h"

//#define SAMPLE_RATE 16000

//#define READ_DEVICE_NAME	"hw:0,3"
//#define READ_DEVICE_NAME 	"microphone"

//#define READ_FRAME 768
//#define BUFFER_SIZE (SAMPLE_RATE/2) //8000
//#define PERIOD_SIZE (BUFFER_SIZE/4) //4000

#if 0
static snd_pcm_t *handle = NULL;
static char record_device_name[32] = {"microphone"};
static int s_sample_rate_hz = 16000;
static int s_channles = 2;
static snd_pcm_format_t s_pcm_format = SND_PCM_FORMAT_S16_LE;
static int s_format_bits = 16;
static audiodata_callback s_callback = NULL;

static pthread_t s_threadid = 0;
static bool s_running = false;
#endif

static void *recorder_getdata(void *param);

/************************************************************
** FunctionName : pcmrecorder_setcallback
** Description  : 设置录音数据回调
** Input Param  : 
				  handle_pcmrecorder hpcmrecorder 
				  audiodata_callback cb
** Output Param :
** Return Value :
**************************************************************/
void pcmrecorder_setcallback(handle_pcmrecorder hpcmrecorder, audiodata_callback cb)
{
	hpcmrecorder->callback = cb;
}


/************************************************************
** FunctionName : pcmrecorder_init
** Description  : 初始化录音模块
** Input Param  : const char *deviceName 设置录音设备名称 例如 hw:0,2
** Output Param :
** Return Value : handle_pcmrecorder:成功  NULL:失败
**************************************************************/
handle_pcmrecorder pcmrecorder_init(const char *deviceName)
{
	handle_pcmrecorder hpcmrecorder = (handle_pcmrecorder)malloc(sizeof(st_pcmrecorder));
	memset(hpcmrecorder, 0, sizeof(st_pcmrecorder));

	int size = sizeof(hpcmrecorder->device_name);
	snprintf(hpcmrecorder->device_name, size - 1, "%s", deviceName);

	hpcmrecorder->channles = 2;
	hpcmrecorder->sample_rate_hz = 16000;
	hpcmrecorder->pcm_format = SND_PCM_FORMAT_S16_LE;
	hpcmrecorder->format_bits = 16;
	hpcmrecorder->callback = NULL;
	hpcmrecorder->threadid = 0;
	hpcmrecorder->running = false;

	/* Open PCM device for recording (capture). */
	/* 打开 PCM capture 捕获设备 */
	int rc = snd_pcm_open(&hpcmrecorder->handle, hpcmrecorder->device_name, SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		LOG(LEVEL_FATAL, "unable to open pcm device(%s): %s", deviceName, snd_strerror(rc));
		free(hpcmrecorder);
		return NULL;
	}

	return hpcmrecorder;
}


/************************************************************
** FunctionName : pcmrecorder_setchannelcount
** Description  : 设置录音通道数
** Input Param  :
					handle_pcmrecorder hpcmrecorder
					int chnlCnt 录音通道数
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_setchannelcount(handle_pcmrecorder hpcmrecorder, int chnlCnt)
{
	hpcmrecorder->channles = chnlCnt;
	return 0;
}



/************************************************************
** FunctionName : pcmrecorder_setformat
** Description  : 设置录音数据格式
** Input Param  :
					handle_pcmrecorder hpcmrecorder
					snd_pcm_format_t format 数据格式
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_setformat(handle_pcmrecorder hpcmrecorder, snd_pcm_format_t format)
{
	hpcmrecorder->pcm_format = format;
	switch(hpcmrecorder->pcm_format) {
		case SND_PCM_FORMAT_S8:
		case SND_PCM_FORMAT_U8:
			hpcmrecorder->format_bits = 8;
			break;
		case SND_PCM_FORMAT_S16_LE:
		case SND_PCM_FORMAT_S16_BE:
		case SND_PCM_FORMAT_U16_LE:
		case SND_PCM_FORMAT_U16_BE:
			hpcmrecorder->format_bits = 16;
			break;
		case SND_PCM_FORMAT_S24_LE:
		case SND_PCM_FORMAT_S24_BE:
		case SND_PCM_FORMAT_U24_LE:
		case SND_PCM_FORMAT_U24_BE:
			hpcmrecorder->format_bits = 24;
			break;
		case SND_PCM_FORMAT_S32_LE:
		case SND_PCM_FORMAT_S32_BE:
		case SND_PCM_FORMAT_U32_LE:
		case SND_PCM_FORMAT_U32_BE:
			hpcmrecorder->format_bits = 32;
			break;
		default:
			LOG(LEVEL_ERROR, "format unknow...");
			return -1;
	}

	return 0;
}



/************************************************************
** FunctionName : pcmrecorder_setsamplerate
** Description  : 设置录音采样率(单位Hz)
** Input Param  :
					handle_pcmrecorder hpcmrecorder
					int sample_rate_hz 采样率(单位Hz)
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_setsamplerate(handle_pcmrecorder hpcmrecorder, int sample_rate_hz)
{
	hpcmrecorder->sample_rate_hz = sample_rate_hz;
	return 0;
}


/************************************************************
** FunctionName : pcmrecorder_start
** Description  : 启动录音 (与Stop配对使用，Stop之后可再次Start)
** Input Param  : handle_pcmrecorder hpcmrecorder
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_start(handle_pcmrecorder hpcmrecorder)
{
	LOG(LEVEL_INFO, "");
	if(hpcmrecorder->handle == NULL) {
		LOG(LEVEL_ERROR, "hpcmrecorder->handle is NULL");
		return -1;
	}

	hpcmrecorder->running = true;
#if 0
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	param.sched_priority = 80;
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	int ret = pthread_create(&hpcmrecorder->threadid, &attr, recorder_getdata, (void*)hpcmrecorder);
#else
	int ret = pthread_create(&hpcmrecorder->threadid, NULL, recorder_getdata, (void*)hpcmrecorder);
#endif

	LOG(LEVEL_INFO, "record thread start...");
	if(ret != 0 ) {
		hpcmrecorder->running = false;
		return -1;
	}

	return 0;
}


/************************************************************
** FunctionName : pcmrecorder_stop
** Description  : 停止录音 (与Start配对使用，Stop之后可再次Start)
** Input Param  : handle_pcmrecorder hpcmrecorder
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_stop(handle_pcmrecorder hpcmrecorder)
{
	LOG(LEVEL_INFO, "");

	if(hpcmrecorder->running) {
		hpcmrecorder->running = false;
		if(hpcmrecorder->threadid > 0) {
			pthread_join(hpcmrecorder->threadid, NULL);
		}
	}
	LOG(LEVEL_INFO, "");
	return 0;
}


/************************************************************
** FunctionName : pcmrecorder_uninit
** Description  : 反初始化
** Input Param  : handle_pcmrecorder hpcmrecorder
** Output Param :
** Return Value : 0:成功  other:失败
**************************************************************/
int pcmrecorder_uninit(handle_pcmrecorder hpcmrecorder)
{
	LOG(LEVEL_INFO, "");

	if(hpcmrecorder->running) {
		hpcmrecorder->running = false;
		if(hpcmrecorder->threadid > 0) {
			pthread_join(hpcmrecorder->threadid, NULL);
		}
	}

	if(hpcmrecorder->handle != NULL) {
		snd_pcm_drop(hpcmrecorder->handle);
		snd_pcm_close(hpcmrecorder->handle);
		hpcmrecorder->handle = NULL;
	}

	free(hpcmrecorder);
	return 0;
}


void *recorder_getdata(void *param)
{
	handle_pcmrecorder hpcmrecorder = (handle_pcmrecorder)param;
	int rc = 0;
	unsigned int size = 0;

	snd_pcm_hw_params_t *params;
	unsigned int val = 0;
	int dir;
	snd_pcm_uframes_t frames;
	unsigned char *buffer = NULL;

	/* Allocate a hardware parameters object. */
	/* 分配一个硬件参数结构体 */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	/* 使用默认参数 */
	snd_pcm_hw_params_any(hpcmrecorder->handle, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	snd_pcm_hw_params_set_access(hpcmrecorder->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	/* Signed 16-bit little-endian format */
	/* 16位 小端 */
	snd_pcm_hw_params_set_format(hpcmrecorder->handle, params, hpcmrecorder->pcm_format);

	/* Two channels (stereo) */
	/* 8通道 */
	snd_pcm_hw_params_set_channels(hpcmrecorder->handle, params, hpcmrecorder->channles);

	/* 44100 bits/second sampling rate (CD quality) */
	/* 采样率 */
	val = hpcmrecorder->sample_rate_hz;
	snd_pcm_hw_params_set_rate_near(hpcmrecorder->handle, params, &val, &dir);

#if 0
	/* Set period size to 32 frames. */
	/* 一个周期有 32 帧 */
	frames = 4096;
	snd_pcm_hw_params_set_period_size_near(hpcmrecorder->handle, params, &frames, &dir);
#else
	{
		unsigned buffer_time = 0;
		unsigned period_time = 0;
		snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
		if(buffer_time > 500 * 1000) {
			buffer_time = 500 * 1000;
		}

		period_time = buffer_time / 4;
		snd_pcm_hw_params_set_period_time_near(hpcmrecorder->handle, params, &period_time, 0);
		snd_pcm_hw_params_set_buffer_time(hpcmrecorder->handle, params, buffer_time, 0);
	}
#endif

	//frames = 4096;
	//snd_pcm_hw_params_set_period_size_near(hpcmrecorder->handle, params, &frames, &dir);


	/* Write the parameters to the driver */
	/* 参数生效 */
	rc = snd_pcm_hw_params(hpcmrecorder->handle, params);
	if (rc < 0) {
		LOG(LEVEL_FATAL, "unable to set hw parameters: %s", snd_strerror(rc));
		//fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		exit(1);
	}

	/* Use a buffer large enough to hold one period */
	/* 得到一个周期的数据大小 */
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	LOG(LEVEL_INFO, "get period_size %d frames", (int)frames);

	/* 16位 双通道，所以要 *4 */
	size = frames * hpcmrecorder->format_bits / 8 * hpcmrecorder->channles; /* 2 bytes/sample, 2 channels */
	LOG(LEVEL_INFO, "frames:%d format_bits:%d channles:%d size:%d", \
	    (int)frames, hpcmrecorder->format_bits, hpcmrecorder->channles, size);

	buffer = (unsigned char *) malloc(size);
	if(buffer == NULL) {
		LOG(LEVEL_FATAL, "malloc(%d) failed!", size);
		//fprintf(stderr, "malloc(%d) failed!\n", size);
		exit(1);
	}
	memset(buffer, 0, size);

	snd_pcm_hw_params_get_period_time(params, &val, &dir);
	LOG(LEVEL_INFO, "snd_pcm_hw_params_get_period_time %d us", val);
#if 0
	{
		snd_output_t *log;
		snd_output_stdio_attach(&log, stderr, 0);
		snd_pcm_dump(hpcmrecorder->handle, log);
		snd_output_close(log);
	}
#endif
	long long errCount = 0LL;

	while (hpcmrecorder->running) {

		/* 捕获数据 */
		rc = snd_pcm_readi(hpcmrecorder->handle, buffer, frames);

		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			if(errCount++ % 200LL == 0) {
				LOG(LEVEL_ERROR,  "overrun occurred");
			}
			snd_pcm_prepare(hpcmrecorder->handle);

		} else if (rc < 0) {
			if(errCount++ % 1000LL == 0) {
				LOG(LEVEL_ERROR, "error from read: %s",  snd_strerror(rc));
				//fprintf(stderr, "%s %d error from read: %s\n", __func__, __LINE__,  snd_strerror(rc));
			}
			usleep(1 * 1000);
			continue;
		} else if (rc != (int)frames) {
			#if 0
			if(errCount++ % 200LL == 0) {
				//fprintf(stderr, "short read, read %d frames\n", rc);
				LOG(LEVEL_ERROR, "short read, read %d frames", rc);
			}
			#endif
		}

		if(hpcmrecorder->callback != NULL && hpcmrecorder->running) {
			hpcmrecorder->callback((unsigned char *)buffer, size, (int)rc);
		}
	}

	LOG(LEVEL_INFO, "snd_pcm_readi...loop out");
	free(buffer);
	return (void*)0;
}



