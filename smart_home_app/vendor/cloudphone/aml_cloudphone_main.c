/***************************************************************************
** CopyRight: Amlogic
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-10
** Description
**
***************************************************************************/
#include "common.h"

#include "gatplatform.h"
//#include "homeserver.h"
#include "json_engine.h"
//#include "tcp.h"
//#include <cloudphone.h>
//#include <netinet/tcp.h>
#include "statusevent_manage.h"

#include "aml_cloudphone_main.h"
#include "aml_cloudphone_api.h"
//#include "ifly_cae_main.h"
//#include "sai_audio_main.h"
#include "sai_voip_imp.h"

#define LOG_TAG "aml_cloudphone_main"

#if 1 //cloudphone
pthread_mutex_t cloudphone_mutex;
int cloudphone_cmd;
pthread_cond_t  cloudphone_condition;  
pthread_t cloudphone_ThreadId=0;;
int g_cloudphone_ok;

phoneNode_t *pphoneNode = NULL;
static phoneNode_t phoneData;

uint8 sys_volume_tab[11]={0,130,150,170,185,200,215,226,236,244,250};
int volueIndex = 6;
static int phone_volueIndex = 6;
static const int phone_volue_max =6;

char notifyWords[128];

static gatTimer_st PhoneRingTimer;

extern int g_cloudphone_session;

gatTimer_st* getPhoneRingTimer(void)
{
	return &PhoneRingTimer;
}

static int update_volue(void)
{
	char buf[128];
	int temp_volueIndex = 6;
	if(isCloudPhoneBusy()||isBtPhoneBusy())
		temp_volueIndex=phone_volueIndex;
	else
		temp_volueIndex =volueIndex;

	sprintf(buf,"/etc/adckey/adckey_function.sh homeServerVolume %d", sys_volume_tab[temp_volueIndex]);
	system(buf);
	return 0;
}

void callingEventHandle(phoneNode_t* pPhoneNode) 
{
	int ret;
	int32 len = 0;
	int32 type;
	char contactId[50]={0};
	char user_phone_num[50];
	
	gatPrintf( GAT_DEBUG,"enter %s\n",__func__);
	if(pPhoneNode == NULL)
	{
		acePrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}
	pphoneNode = (phoneNode_t*)pPhoneNode;
	acePrintf( GAT_NOTIC,"%s  phone number : %s\n",__FUNCTION__, pphoneNode->user_phone_num);
	#if 0
	if((strcmp(pphoneNode->contactId, contactId) == 0)&&(strcmp(pphoneNode->user_phone_num, user_phone_num) == 0))
	{
		playermanager_play_text("抱歉，通讯录中没有找到这个人。你可以同步手机通讯录到音箱后再试，或直接说手机号。");
		return;
	}
	#endif
	setDevStatus(DEV_STATUS_CALLING, 1);

	#if 0 //for test
	memcpy(pphoneNode->contactId, "2", 1);
	memset(pphoneNode->user_phone_num, 0, sizeof(pphoneNode->user_phone_num));
	#endif
	
	int count = 0;
	while(pthread_mutex_trylock(&cloudphone_mutex))
	{
		printf("get phone lock error.\n");		
		usleep(100*1000);	
		if (count ++ > 30)
		{
			printf("can't get phone lock.\n");		
			return -1;
		}
	}

	cloudphone_cmd = PHONE_CALLOUT;
	pthread_cond_signal(&cloudphone_condition);
	pthread_mutex_unlock(&cloudphone_mutex);

	printf("############# callingEventHandle ##########\n");
	gatPrintf( GAT_DEBUG,"user_name=%s, user_phone_num=%s\n", pphoneNode->user_name, pphoneNode->user_phone_num);
	gatPrintf( GAT_DEBUG,"contactId=%s, version=%s\n", pphoneNode->contactId, pphoneNode->version);
	printf("###########################################\n");
}

static int search_callback(void * data, int argc, char ** argv, char ** azColName) 
{
	int i;
	printf("%s .....\n", __func__);
	fprintf(stderr, "%s\n", (const char *)data);
	for(i=0; i< argc; i++) {
		printf("%s : %s\n", azColName[i], argv[i]?argv[i]: "NULL");
	}
	sprintf(phoneData.user_name, "%s", argv[0]);
	sprintf(phoneData.user_phone_num, "%s",argv[1]);
	sprintf(phoneData.contactId, "%s",argv[2]);
	sprintf(phoneData.version, "%s",argv[3]);

	return 0;
}

void updateDbStatus(const char* str) 
{
	sqlite3 * db = NULL;
	int result=-1;

	if(SQLITE_OK != sqlite3_open("/data/migubook.db", &db)) {
		gatPrintf( GAT_DEBUG,"open db file failed: %s\n", sqlite3_errmsg(db));
		return;
	}

	gatPrintf( GAT_DEBUG,"updateDbStatus:%s\n", str);
	result = update_recallCallbackNo_to_file(NULL,str,0);
	gatPrintf( GAT_DEBUG,"update_recallCallbackNo_to_file result:%d\n", result);

	search_contact_number(str, db, search_callback);
	usleep(100*1000);
	sqlite3_close(db);
}

static int32 PhoneRingTimerCB( gatTimer_st *pTimer ) 
{
	playermanager_play_file("/media/sound/bt_ring.pcm");
	return 2;
}

/*收到某人的来电*/
void cloudphone_OnRecvCall_CB(int session, const char* from, 
		const char* displayname, const char* to, int callType) 
{
	int32 len = 0;
	int32 type;
	int notify_flag = 0;
	char user_name[50]={0};
	char user_phone_num[50]={0};
	char contactId[50]={0};

	printf("################ cloudphone ###################\n");
	gatPrintf( GAT_DEBUG,"recv call session=%d, from=%s, displayname=%s\n", session, from, displayname );
	gatPrintf( GAT_DEBUG,"to=%s, callType=%d\n", to, callType);
	printf("###############################################\n");

	g_cloudphone_session = session;

	#if 0
	type = TYPE_NOTIFY;
	sendInterruptEvt(type);

	memset(notifyWords , 0, sizeof(notifyWords ));
	
	//updateDbStatus(from);
	#endif
	gatTimerAdd( getPhoneRingTimer(), "PhoneRingTimer",0,PhoneRingTimerCB,NULL);
 
}

/*对方响铃*/
void cloudphone_OnRecvRing_CB(int session, const char* from, 
		const char* displayname, const char* to, int EarlyMedia) {
	printf("################ cloudphone ###################\n");
	gatPrintf( GAT_DEBUG,"on recv ring, session=%d, from=%s, displayname=%s\n", session, from, displayname );
	gatPrintf( GAT_DEBUG,"to=%s, EarlyMedia=%d\n", to, EarlyMedia);
	printf("###############################################\n");

	
	//setDevStatus(DEV_STATUS_CALLING, 1);
}

/*通话被对方挂断*/
void cloudphone_OnRecvHangup_CB(int session, int errorCode, const char* reason) 
{
	printf("################ cloudphone ###################\n");
	gatPrintf( GAT_DEBUG,"recv hangup session=%d, errorCode=%d, reason=%s\n", session, errorCode, reason);
	printf("###############################################\n");


	setDevStatus(DEV_STATUS_HANGUP, 1);
	sai_voip_imp_stop();
	#if 0
	cae_set_phone_mode_wrapper(0);
	caeResetEng();
	#endif
	playermanager_play_text("通话被挂断");
    gatTimerDel( getPhoneRingTimer());
}

void cloudphone_OnRecvHangupAck_CB(int session, int errorCode, const char* reason) 
{
	printf("################ cloudphone ###################\n");
	gatPrintf( GAT_DEBUG,"session=%d, errorCode=%d, reason=%s\n", session, errorCode, reason);
	printf("###############################################\n");


	setDevStatus(DEV_STATUS_HANGUP, 1);
	sai_voip_imp_stop();
	#if 0
	cae_set_phone_mode_wrapper(0);
	caeResetEng();
	#endif
	update_volue();
	playermanager_play_text("电话挂断");
	gatTimerDel( getPhoneRingTimer());
}


/*对方接听时候回调*/
void cloudphone_OnRecvAnswer_CB(int session, const char* from, 
								const char* displayname, const char* to, int callType) 
{
	printf("################ cloudphone ###################\n");
	gatPrintf( GAT_DEBUG,"recv answer session=%d, from=%s, displayname=%s\n", session, from, displayname);
	gatPrintf( GAT_DEBUG,"to=%s, callType=%d\n", to, callType);
	printf("###############################################\n");

	setDevStatus(DEV_STATUS_TALKING, 1);
	//cae_set_phone_mode_wrapper(1);
	//update_volue();
	sai_voip_imp_start();

}

void cloudphone_OnUserLoginSucceed_CB(const char* loginStr)
{
	printf("################ %s ###################\n", __func__);
	gatPrintf( GAT_DEBUG,"OnUserLoginSucceed: %s\n", loginStr);
//	setDevStatus(DEV_STATUS_PHONE_OK,1); 
	g_cloudphone_ok = 1;
	printf("###############################################\n");
}

void cloudphone_OnUserLoginFailed_CB(const char*user, int errorCode, const char* reason) {
	printf("################ %s ###################\n", __func__);
	gatPrintf( GAT_DEBUG,"OnUserLoginFailed: %s, %d, %s\n", user, errorCode, reason);
//	setDevStatus(DEV_STATUS_PHONE_OK,0); 
	g_cloudphone_ok = 0;
	printf("###############################################\n");
	
	setDevStatus(DEV_STATUS_HANGUP, 1);
	#if 0
	cae_set_phone_mode_wrapper(0);
	caeResetEng();
	#endif
	playermanager_play_text("电话挂断");

	if (getDevStatus() & DEV_STATUS_MUTE_PENDING)
	{
		setDevStatus(DEV_STATUS_MUTE_PENDING, 0);
		setDevStatus(DEV_STATUS_MICMUTE, 1);
		//cae_mute(1);
	}
}

void cloudphone_OnUserLogout_CB(const char*user) {
	printf("################ %s ###################\n", __func__);
	gatPrintf( GAT_DEBUG,"OnUserLogout: %s\n", user);
	//setDevStatus(DEV_STATUS_PHONE_OK,0); 
	g_cloudphone_ok = 0;
	printf("###############################################\n");
	
	setDevStatus(DEV_STATUS_HANGUP, 1);
	#if 0
	cae_set_phone_mode_wrapper(0);
	caeResetEng();
	#endif
	playermanager_play_text("电话挂断");

	if (getDevStatus() & DEV_STATUS_MUTE_PENDING)
	{
		setDevStatus(DEV_STATUS_MUTE_PENDING, 0);
		setDevStatus(DEV_STATUS_MICMUTE, 1);
		//cae_mute(1);
	}
}
#define VERSION "2.3.0"

void *cloudphoneThread(void *arg)
{
	sigset_t signal_mask;
	int ret = -1;
	int initCnt = 0;
	miguPrintf(GAT_DEBUG,"%s\n", __func__);
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) == -1) {
		perror("SIGPIPE");
	}

	miguPrintf(GAT_DEBUG,"\n\nbegin cloudphone loop.\n");
	sleep(3);
	ret = cloudphoneInit(VERSION);
	while (1) 
	{
		while (cloudphone_cmd == PHONE_IDLE) {
			//pthread_cond_wait(&cloudphone.condition, &cloudphone.mutex);
			pthread_cond_wait(&cloudphone_condition,
					  &cloudphone_mutex);
		}

		miguPrintf(GAT_DEBUG,"\n\ncloudphone loop get signal:%d\n",
		       cloudphone_cmd);
		switch (cloudphone_cmd) {
		case PHONE_INIT:
			ret = cloudphoneInit(VERSION);
			cloudphone_cmd = PHONE_IDLE;
			break;
		case PHONE_BIND:
			ret = cloudphoneGetAccount(NULL);
			cloudphone_cmd = PHONE_IDLE;
			break;

		case PHONE_UNBIND:
			g_cloudphone_ok = 0;
			ret = cloudphoneReleaseAccount();
			cloudphone_cmd = PHONE_IDLE;
			break;
		case PHONE_CALLOUT:
			sleep(4);
			if (getDevStatus() & DEV_STATUS_HANGUP)
			{
				//cae_set_phone_mode_wrapper(0);
				gatTimerDel(getPhoneRingTimer());
			}
			else
			{
				ret =
					cloudphoneCall(pphoneNode->user_phone_num,
						   pphoneNode->contactId, 20,
						   &g_cloudphone_session);
				if (ret < 0) {
					setDevStatus(DEV_STATUS_HANGUP, 1);
					//ledControl(MIGU_FEN_CHENG, 0, 0);
					//splayer_play_text("呼叫失败", 1);
					//cae_set_phone_mode_wrapper(0);
					//caeResetEng();
					gatTimerDel(getPhoneRingTimer());
				}
			}
			cloudphone_cmd = PHONE_IDLE;
			break;

		case PHONE_DEREGIST:
			g_cloudphone_ok = 0;
			cloudphoneRelease(0);
			cloudphone_cmd = PHONE_IDLE;
			break;
		default:
			break;
		}
	}

	g_cloudphone_ok = 0;
	cloudphoneRelease(0);
	pthread_exit((void *) 0);
}


int aml_enterCloudphone(void)
{
	int ret = -1;
	printf("%s\n",__func__); 
	if (cloudphone_ThreadId == 0)
	{
		cloudphone_onRecvCall_setCallback(cloudphone_OnRecvCall_CB);
		cloudphone_onRecvRing_setCallback(cloudphone_OnRecvRing_CB);
		cloudphone_onRecvHangup_setCallback(cloudphone_OnRecvHangup_CB);
		cloudphone_onRecvHangupAck_setCallback(cloudphone_OnRecvHangupAck_CB);
		cloudphone_onRecvAnswer_setCallback(cloudphone_OnRecvAnswer_CB);
		cloudphone_onLoginSucced_setCallback(cloudphone_OnUserLoginSucceed_CB);
		cloudphone_onLoginFailed_setCallback(cloudphone_OnUserLoginFailed_CB);
		cloudphone_onLoginOut_setCallback(cloudphone_OnUserLogout_CB);
		//memset(&p_cloudphone, 0, sizeof(CloudPhone));
		//pthread_cond_init(&cloudphone.condition, NULL);
		//pthread_mutex_init(&cloudphone.mutex,NULL);

		pthread_cond_init(&cloudphone_condition, NULL);
		pthread_mutex_init(&cloudphone_mutex,NULL);

		pthread_mutex_lock(&cloudphone_mutex);
		cloudphone_cmd = PHONE_BIND;
		pthread_mutex_unlock(&cloudphone_mutex);

		ret = iofPthreadCreate(&cloudphone_ThreadId, NULL, cloudphoneThread, NULL);
		if(ret != 0)
		{
			gatPrintf( GAT_ERROR,"can't creat thread.err:%s\r\n", strerror(ret));
			return -1;
		}

	}
	else
	{
		cloudphone_cmd_triger(PHONE_BIND);
	}

	if(cloudphone_ThreadId!= 0)
	{
		pthread_detach(cloudphone_ThreadId);
		//usleep(200*1000);
	}
	else
	{
		perror("can't creat thread.err:");
		return -1;
	}
	return 0;
}
#endif

