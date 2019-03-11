/***************************************************************************
** CopyRight: Amlogic
** Author   : jian.cai@amlogic.com
** Date     : 2018-09-30
** Description
**
***************************************************************************/
#ifndef _AML_TTS_INTERFACE_H_
#define _AML_TTS_INTERFACE_H_

typedef enum  {
	em_ttsplay_event_finish,
	em_ttsplay_event_error
} ttsplay_event;

typedef void (*ttsplay_event_callback)(ttsplay_event event, void *eventparam, void *userparam);

/*
tts 播放接口类
*/
class aml_tts_interface
{
public:
	virtual void init(void *param) = 0;

	virtual void setcallback(ttsplay_event_callback cb, void *userparam) = 0;
	virtual void play(const char *text) = 0;
	virtual void waitfinish(int timeout) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void stop() = 0;

	virtual void uninit(void *param) = 0;

	virtual ~aml_tts_interface() {};
};

#endif

