/***************************************************************************
** CopyRight: Amlogic
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-28
** Description
**
***************************************************************************/
#define LOG_TAG "aml_gst_player"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gst/gst.h>

#include "aml_log.h"
#include "aml_gst_player_imp.h"

#if 0
static GMainLoop *g_pLoop = NULL;
static GstElement *g_pPlaypipe = NULL;
static GstBus *g_pBus = NULL;
static pthread_t mainthread;
static aml_gstplayer_status g_status = EM_GSTPLAYER_STATUS_UNKNOWN;
static aml_gstplayer_event_cb g_event_cb = NULL;
static aml_gstplayer_onprogress g_on_progress = NULL;
#endif

struct _st_gstplayer {
	GMainLoop *pLoop;
	GstElement *pPlaypipe;
	GstBus *pBus;
	pthread_t mainthread;
	aml_gstplayer_status status;
	aml_gstplayer_event_cb event_cb;
	aml_gstplayer_onprogress on_progress;
};

static st_gstplayer *g_pgstplayer = NULL;

static void do_exit(st_gstplayer *pgstplayer)
{
	gst_bus_remove_signal_watch(pgstplayer->pBus);
	gst_element_set_state (pgstplayer->pPlaypipe, GST_STATE_NULL);
	g_object_unref(pgstplayer->pBus);
	g_object_unref (pgstplayer->pPlaypipe);
	g_main_loop_unref(pgstplayer->pLoop);
}


static void error_quit(GstBus *bus, GstMessage *message, st_gstplayer *pgstplayer)
{
	GError *error;
	gst_message_parse_error(message, &error, NULL);
	LOG(LEVEL_ERROR, "%s %d Error:%s\n", __func__, __LINE__, error->message);
	g_error_free(error);
	
	gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_NULL);
	pgstplayer->status = EM_GSTPLAYER_STATUS_STOPPED;

	if(pgstplayer->event_cb != NULL) {
		pgstplayer->event_cb(pgstplayer, EM_GSTPLAYER_EVENT_PlayError, NULL);
	}
}


static void end_of_streamer(GstBus *bus, GstMessage *message, st_gstplayer *pgstplayer)
{
	gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_NULL);

	pgstplayer->status = EM_GSTPLAYER_STATUS_STOPPED;

	if(pgstplayer->event_cb != NULL) {
		pgstplayer->event_cb(pgstplayer, EM_GSTPLAYER_EVENT_PlayFinished, NULL);
	}
}


static void *task_main_loop(void *param)
{
	st_gstplayer *pgstplayer = (st_gstplayer *)param;
	
	g_main_loop_run(pgstplayer->pLoop);
	return NULL;
}


static void get_progress(st_gstplayer *pgstplayer)
{
	if(pgstplayer->status != EM_GSTPLAYER_STATUS_PLAYING  
		&&  pgstplayer->status != EM_GSTPLAYER_STATUS_PAUSE){
		return;
	}

	GstFormat fm = GST_FORMAT_TIME;
	gint64 pos = 0;
	gint64 len = 0;

	gst_element_query_position(pgstplayer->pPlaypipe, fm, &pos);
	gst_element_query_duration(pgstplayer->pPlaypipe, fm, &len);

	unsigned int cur_pos = (unsigned int)(pos / 1000000);
	unsigned int duration = (unsigned int)(len / 1000000);

	//printf("\n%s %d cur_pos:%u duration:%u\n", __func__, __LINE__, cur_pos, duration);

	if((duration > 0) && (pgstplayer->on_progress != NULL)) {
		pgstplayer->on_progress(pgstplayer, cur_pos, duration);
	}
}


st_gstplayer *aml_gstplayer_init(aml_gstplayer_event_cb event_cb, aml_gstplayer_onprogress OnProgress)
{
	st_gstplayer *pgstplayer = (st_gstplayer*)malloc(sizeof(st_gstplayer));
	if(pgstplayer == NULL){
		return NULL;
	}
	
	memset(pgstplayer, 0, sizeof(st_gstplayer));
	
	pgstplayer->event_cb = event_cb;
	pgstplayer->on_progress = OnProgress;

	pgstplayer->status = EM_GSTPLAYER_STATUS_UNKNOWN;

	gst_init(NULL, NULL);
	pgstplayer->pLoop = g_main_loop_new(NULL, FALSE);

	pgstplayer->pPlaypipe = gst_element_factory_make("playbin", NULL);

	pgstplayer->pBus = gst_pipeline_get_bus(GST_PIPELINE(pgstplayer->pPlaypipe));
	gst_bus_add_signal_watch(pgstplayer->pBus);

	g_signal_connect(G_OBJECT(pgstplayer->pBus), "message::error", G_CALLBACK(error_quit), pgstplayer);
	g_signal_connect(G_OBJECT(pgstplayer->pBus), "message::eos", G_CALLBACK(end_of_streamer), pgstplayer);
	
	g_timeout_add(1000, (void *)get_progress, pgstplayer);

	pthread_create(&pgstplayer->mainthread, NULL, task_main_loop, (void*)pgstplayer);

	pgstplayer->status = EM_GSTPLAYER_STATUS_INIT;

	return pgstplayer;
}


/************************************************************
** FunctionName : aml_gstplayer_play
** Description  : 播放url  ，例如 file:///data/test.mp3    ,  http://xxx.xxx.xxx/test.mp3
** Input Param  :
					st_gstplayer *pgstplayer
					const char *pUrl
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_play(st_gstplayer *pgstplayer, const char *pUrl)
{
	gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_READY);
	g_object_set(G_OBJECT(pgstplayer->pPlaypipe), "uri", pUrl, NULL);
	gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_PLAYING);
	pgstplayer->status = EM_GSTPLAYER_STATUS_PLAYING;
	return 0;
}


/************************************************************
** FunctionName : aml_gstplayer_pause
** Description  : 暂停
** Input Param  : st_gstplayer *pgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_pause(st_gstplayer *pgstplayer)
{
	gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_PAUSED);
	pgstplayer->status = EM_GSTPLAYER_STATUS_PAUSE;
	return 0;
}


/************************************************************
** FunctionName : aml_gstplayer_resume
** Description  : 暂停恢复
** Input Param  : st_gstplayer *pgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_resume(st_gstplayer *pgstplayer)
{
	gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_PLAYING);
	pgstplayer->status = EM_GSTPLAYER_STATUS_PLAYING;
	return 0;
}


/************************************************************
** FunctionName : aml_gstplayer_stop
** Description  : 停止
** Input Param  : st_gstplayer *pgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_stop(st_gstplayer *pgstplayer)
{
	gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_NULL);
	pgstplayer->status = EM_GSTPLAYER_STATUS_STOPPED;
	return 0;
}


/************************************************************
** FunctionName : aml_gstplayer_seek
** Description  : 从指定位置开始播放
** Input Param  : 
					st_gstplayer *pgstplayer
					int pos，单位ms
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_seek(st_gstplayer *pgstplayer, int pos)
{
	gboolean ret = gst_element_seek(pgstplayer->pPlaypipe, 1, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
	                                GST_SEEK_TYPE_SET, pos * GST_MSECOND,
	                                GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE);

	LOG(LEVEL_INFO, "gst_element_seek=%d\n", ret);
	return 0;
}


/************************************************************
** FunctionName : aml_gstplayer_get_duration
** Description  : 获取时长，单位ms
** Input Param  : st_gstplayer *pgstplayer
** Output Param :
** Return Value : 时长单位ms， 失败返回0
**************************************************************/
int aml_gstplayer_get_duration(st_gstplayer *pgstplayer)
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 m_length;
	gst_element_query_duration(pgstplayer->pPlaypipe, fmt, &m_length);

	return (unsigned int)(m_length / 1000000);
}


/************************************************************
** FunctionName : aml_gstplayer_get_status
** Description  : 获取状态
** Input Param  : st_gstplayer *pgstplayer
** Output Param :
** Return Value : aml_gstplayer_status
**************************************************************/
aml_gstplayer_status aml_gstplayer_get_status(st_gstplayer *pgstplayer)
{
	return pgstplayer->status;
}


/************************************************************
** FunctionName : aml_gstplayer_uninit
** Description  : 反初始化
** Input Param  : st_gstplayer *pgstplayer
** Output Param :
** Return Value : 0:success   other:failed
**************************************************************/
int aml_gstplayer_uninit(st_gstplayer *pgstplayer)
{
	LOG(LEVEL_INFO, "pgstplayer=%p g_pgstplayer=%p", pgstplayer, g_pgstplayer);
	
	if(g_main_loop_is_running(pgstplayer->pLoop)) {
		gst_element_set_state(pgstplayer->pPlaypipe, GST_STATE_NULL);
		g_main_loop_quit(pgstplayer->pLoop);
	}

	pthread_join(pgstplayer->mainthread, NULL);
	do_exit(pgstplayer);
	
	pgstplayer->status = EM_GSTPLAYER_STATUS_UNKNOWN;

	free(pgstplayer);
	LOG(LEVEL_INFO, "done");
	return 0;
}


