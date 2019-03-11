#define LOG_TAG "main"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <netdb.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <sys/prctl.h>
#include <sys/msg.h>
#include "gatplatform.h"
#include "gatgobal.h"
#include "homeserver.h"
#include "iofipc.h"

#include "aml_log.h"
#include "util.h"
#include "aml_gst_player_imp.h"
#include "aml_tts_play.h"
//#include "ifly_tts_play.h"
//#include "ifly_cae_main.h"
//#include "sai_audio_main.h"
#include "iofpthread.h"
#include "ioftime.h"
#include "timer_subsystem.h"
//#include "ledring.h"
#include "key_event_handler.h"
#include "aml_playermanager.h"
#include "aml_cloudphone_main.h"

#include "sai_voip_imp.h"

static gatpthread_t homeserverTid;

#define VoIP_FIFO "/tmp/voip_fifo"
int fifo_writer_fd=0;
static int writeCnt=0;

static void del_semvalue()
{
	if(msgctl(getMsgControlId(), IPC_RMID, 0 ) == -1) {
		fprintf(stderr, "Failed to delete semaphore\n");
	}
	fflush(stdout );
	exit(0);
}

void clientExit(int sig)
{
	printf("\n%s... now!\n", __FUNCTION__ );
	del_semvalue();
}

static bool wakeup_callback(unsigned angle, void *user_param) 
{
	LOG(LEVEL_INFO, "--------------------Wake up---------------------------------");
	LOG(LEVEL_INFO, "angle: %d", angle);
	LOG(LEVEL_INFO, "------------------------------------------------------------");

	return true;
}

static bool audiodata_callback(const void *buf, int size, void *user_param)
{
	/*
	If bluetooth phone working: 
	    return true
	else 
	   return false 
	*/
	int nwrite;

	if (access(VoIP_FIFO , F_OK) == 0){
		if(1){ //(getDevStatus() & DEV_STATUS_TALKING)
			if (fifo_writer_fd <= 0)
			{
				fifo_writer_fd = open(VoIP_FIFO ,O_RDWR|O_NONBLOCK,0);
			}
			if((nwrite=write(fifo_writer_fd , buf, size))==-1)
			{
				if(errno==EAGAIN)
				{
					if(writeCnt++>2)
					{
						close(fifo_writer_fd); 
						fifo_writer_fd= 0;
						writeCnt=0;
						unlink(VoIP_FIFO);
					}
					printf("The FIFO has not been read yet.Please try later\n" );
				}
			}
			//printf(">>>>>>>>>>>%d\n", nwrite);
			return false;
		}
	}
	else{
		if(fifo_writer_fd > 0){
			close(fifo_writer_fd); 
			fifo_writer_fd= 0; 
		}
	}

	return false;
}

static aml_tts_interface *pttsplayer = NULL;

static void audio_to_text(const char *result, void *user_param)
{
	uint32 evt = EVT_IAT_SEMANTIC;
	recNode_t* pRecNode = evt_get_recNode();
	LOG(LEVEL_INFO, "------------------------------------------------------------");
	LOG(LEVEL_INFO, "%s", result);
	LOG(LEVEL_INFO, "------------------------------------------------------------");

	#if 0
	if(pttsplayer == NULL){
		pttsplayer = new ifly_tts_play();
	}

	if(pttsplayer != NULL){
		pttsplayer->init(NULL);
		pttsplayer->play(result);
		pttsplayer->waitfinish(0);
		pttsplayer->uninit(NULL);
	}else{
		LOG(LEVEL_ERROR, "pttsplayer is NULL");
	}
	#else

	if(strlen(result)<=0)
	{
		LOG(LEVEL_ERROR, "result null err");
		return;
	}
	iofLock((lock_t*)&pRecNode->recLock);
	if(pRecNode->data != NULL)
	{
		memset(pRecNode->data,0,SEMANTIC_BUF_SIZE);
	}
	else
	{
		pRecNode->data = (int8*)iofMalloc(SEMANTIC_BUF_SIZE);
		if(NULL == pRecNode->data)
		{
			LOG(LEVEL_ERROR, "no mem for iofMalloc rec node\r\n");
			iofUnlock((lock_t*)&pRecNode->recLock);
			return;
		}
		memset(pRecNode->data,0,SEMANTIC_BUF_SIZE);
	}	
	if(strlen(result)>=SEMANTIC_BUF_SIZE)
	{
		LOG(LEVEL_ERROR, "no mem for iofMalloc rec node\r\n");
		iofUnlock((lock_t*)&pRecNode->recLock);
		return;
	}
	memcpy(pRecNode->data, (char*)result, strlen(result));
	pRecNode->type = TYPE_SEMANTIC;
	pRecNode->len = strlen(result);
	statusEventNotify(evt, (recNode_t**)&pRecNode);
	iofUnlock((lock_t*)&pRecNode->recLock);
	return;

	#endif
}

void playermanager_callback(em_pm_playertype type, em_pm_playerevent event)
{
	LOG(LEVEL_ERROR, "type:%d event:%d", type, event);
}


static void sound_ai_voip_data_cb(const char *buffer, size_t size)
{
	audiodata_callback((const void *)buffer, (int)size, (void *)NULL);
}

int main(int argc, char **argv)
{
	int32 iRet;
	iofSig_t sig;
	int32 cmd;

	LOG(LEVEL_INFO, "main start....");

	signal(SIGINT, clientExit );
	iofMsgCreat(GAT_KEYID_MSG);
	gatTimerCoreInit();
	iofTimerInit();
	evtDevStatusInit();
	evt_recNode_init();
	//ledInit();
	//led_all_off_mute();
	key_event_handle_start();
	aml_enterCloudphone();

	//aml_tts_interface *pttsplayer = new ifly_tts_play();
	//playermanager_init();
	//playermanager_set_ttsplayer(pttsplayer);
	//playermanager_set_eventcallback(playermanager_callback);


	/*----sound ai voip ------------------------------------------*/
//	sai_voip_imp_init("/etc/sai_config");
//	sai_voip_imp_setcallback(sound_ai_voip_data_cb);
	/*------------------------------------------------------------*/


	iRet = iofPthreadCreate(&homeserverTid, NULL, homeserverThread, NULL);
	if(iRet != 0) {
		LOG(LEVEL_INFO, "can't creat thread.err:%s\r\n", strerror(iRet));
	}
	while(1) {
		iRet = iofRecSig(getMsgControlId(), &sig);
		if(iRet < 0) {
			continue;
		}
		
		LOG(LEVEL_INFO, "rcv msg len:%d\r\n", iRet);
		//dumpBuf( GAT_DEBUG,((char *)&sig),iRet );
		
		switch(sig.cmd) {
			case SIG_CMD_EVT:
				statusEvtHandle(&sig);
				break;
			
			default:
				LOG(LEVEL_WARN, "don't support this cmd:0x%x\r\n", sig.cmd);
				break;
		}
	}
	//led_deInit();
	pthread_join(homeserverTid, NULL);
	sleep(1);
	return 0;
}

