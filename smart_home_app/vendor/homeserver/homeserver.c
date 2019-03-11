/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-30
** Description 
**  
***************************************************************************/
#define LOG_TAG "homeserver"

#include "gatplatform.h"
#include "homeserver.h"
#include "json_engine.h"
#include "tcp.h"
#include <cloudphone.h>
#include <netinet/tcp.h>
#include "migu.h"
#include "aml_playermanager.h"
#include "aml_log.h"


static miguClientNode_t *clientNode = NULL;
static gatTimer_st heartbeatTimer;
static gatTimer_st upgradeInfoTimer;
static gatTimer_st HomeDemoTimer;

gatTimer_st ledTimer;
static int8 bootFlag=0;
static gatTimer_st bootFlagTimer;
static gatTimer_st netconfigTimer;
static gatTimer_st clockTimer;
static gatTimer_st reminderTimer;
static gatTimer_st getPhoneListTimer;
static gatTimer_st requestTimer;
static gatTimer_st ap_config_TimeoutTimer;
extern songNode_t songData;
extern aiMessage_t aiMessage;
extern char time_str[21];
extern int alarm_OK;

int8* msgType [] = {
       "login",
       "heartBeat",
       "pushDevInfo",
       "semantic",
       "getImsInfo",
       "getImsPhoneList",
       "pushAiMessage",
       "getMdevUser",
       "pushMusics",
       "pushControl",
       "syncImsInfo",
       "syncImsBindStatus",
       "syncImsPhoneList",
       "pushMdevUser",
       "getMusicInfoById",
	   "getDevInfo",
	   "getUpgradeInfo",
};

int miguHeartbeat(int32 fd);
char* songURL = NULL;
static int _player_run_flag = 0;
int g_cloudphone_session;

uint32 parsingLen(const unsigned char* buf)
{
	uint32 len = 0;
	len = buf[0] <<24 |buf[1] <<16|buf[2] <<8|buf[3];

    return len;
}

gatTimer_st* getAPConfigTimeoutTimer(void)
{
	return &ap_config_TimeoutTimer;
}

miguClientNode_t* getMiguClientNode(void)
{
	return clientNode;
}

int8 getBootFlag(void)
{
	return bootFlag;
}

void setBootFlag(int8 flag)
{
	bootFlag = flag;
}

static int8 getClientStep(void)
{
	int8 step = 0;
	iofLock(&getMiguClientNode()->cliLock);
	step = getMiguClientNode()->step;
	iofUnlock(&getMiguClientNode()->cliLock);
	return step;
}

int32 getClientFd(void)
{
	int32 fd = 0;
	iofLock(&getMiguClientNode()->cliLock);
	fd = getMiguClientNode()->fd;
	iofUnlock(&getMiguClientNode()->cliLock);
	return fd;
}

void setClientStep(int8 step)
{
	iofLock(&getMiguClientNode()->cliLock);
	getMiguClientNode()->step=step;
	iofUnlock(&getMiguClientNode()->cliLock);
	return;
}

void setClientFd(int32 fd)
{
	iofLock(&getMiguClientNode()->cliLock);
	getMiguClientNode()->fd=fd;
	iofUnlock(&getMiguClientNode()->cliLock);
	return;
}

gatTimer_st* getHeartbeatTimer(void)
{
	return &heartbeatTimer;
}
gatTimer_st* getLEDTimer(void)
{
	return &ledTimer;
}
gatTimer_st* getBootFlagTimer(void)
{
	return &bootFlagTimer;
}

gatTimer_st* getGetPhoneListTimer(void)
{
	return &getPhoneListTimer;
}

gatTimer_st* getNetconfigTimer(void)
{
	return &netconfigTimer;
}

gatTimer_st* getRequestHandleTimer(void)
{
	return &requestTimer;
}
gatTimer_st* getUpgradeInfoTimer(void)
{
	return &upgradeInfoTimer;
}

gatTimer_st* getHomeDemoTimer(void)
{
	return &HomeDemoTimer;
}
int32 bootFlagTimerCb( gatTimer_st *pTimer )
{	
	int32 ret=-1;

	setBootFlag(0);
	return 0;
}

static int32 requestHandleTimerCb( gatTimer_st *pTimer )
{	
	int32 type;
	LOG(LEVEL_INFO, "%s, coming!  \r\n",__FUNCTION__);
	
	type = TYPE_NOTIFY;
	sendInterruptEvt(type);

	playermanager_play_file("/media/sound/network_timeout.pcm");
	
	return 0;
}

static int32 heartbeatTimerCb( gatTimer_st *pTimer )
{	
	int32 ret=-1;
	int32 fd = (int32)pTimer->param;
	miguPrintf( GAT_DEBUG,"fd:%d.\n", pTimer->param);
	LOG(LEVEL_INFO, "%s, coming!  \r\n",__FUNCTION__);

	if(fd < 0)
	{
		miguPrintf( GAT_ERROR,"%s fd invalid!\r\n",__FUNCTION__ );
		return 0;
	}
	ret = miguHeartbeat((int32)pTimer->param);
	if(ret < 0)
	{
		return 0;
	}
	return MIGU_HEARTBEAT_FREQ;
}

int package_json_pushaiMessage_action()
{
	struct RequestJsonStruct request_json_struct;

	struct json_object* json_data, *p_data, *p_raw;
	char * json_string;
	int ret;
	int32 type=MIGU_COMMUNICATION;

	aiMessage_t* p_aiMessage = &aiMessage;
	if (p_aiMessage->result == NULL )
	{
		printf("%d:%s p_aiMessage->result == NULL\n",__func__,__LINE__);
	}

	//put msgType
	request_json_struct.msgType = msgType[PUSH_AI_MESSAGE];

	//put data
	p_data = json_object_new_object();

	json_object_object_add(p_data, "serviceCode", json_object_new_string(p_aiMessage->serviceCode));
	json_object_object_add(p_data, "inputAsrText", json_object_new_string(p_aiMessage->inputAsrText));
	json_object_object_add(p_data, "answerText", json_object_new_string(p_aiMessage->answerText));
	json_object_object_add(p_data, "result", p_aiMessage->result);

	request_json_struct.data = p_data;

	request_json_struct.raw = NULL;
	//make json_data
	json_data = json_make_request_data(&request_json_struct);

	//check json_data
	if (NULL == json_data) {
		gatPrintf( GAT_DEBUG,"make json_data object failed.\n");
		return -1;
	}

	json_string = (char*)json_object_to_json_string(json_data);
	int dataLen = strlen(json_string);
	printf("%s:%d: %s\n ##### Len:%d \n",__func__,__LINE__,json_string,dataLen);
	ret = send_server_data(json_string, type);
	json_object_put(json_data);
	miguPrintf( GAT_DEBUG,"%s data send return %d\n", __func__,ret);

	return 0;

}

int package_json_pushdevinfo_action(void)
{
	struct json_object* json_data, *p_data, *p_raw;
	struct RequestJsonStruct request_json_struct;
    struct tm *tblock;
	char date[100];
	char tmp_buf[50];
	time_t timer;
	int ret;
	int m_current = 0;
	int m_total = 0;
	char * json_string;
	int32 type=MIGU_PUSHDEVINFO;

	songNode_t* p_songNode = &songData;

	/*make push dev info str*/
	//put msgType
	request_json_struct.msgType = msgType[PUSH_DEV_INFO];

	//put data
	p_data = json_object_new_object();

	if(p_songNode->playid == NULL)
	{
		json_object_object_add(p_data, "volume", json_object_new_int(p_songNode->volume));
	}
	else
	{
		json_object_object_add(p_data, "playid", json_object_new_string(p_songNode->playid));
		json_object_object_add(p_data, "song_name", json_object_new_string(p_songNode->song_name));
		json_object_object_add(p_data, "singerName", json_object_new_string(p_songNode->singerName));
		json_object_object_add(p_data, "playMode", json_object_new_int(p_songNode->playMode ));
		json_object_object_add(p_data, "playing", json_object_new_int(p_songNode->playing));
		json_object_object_add(p_data, "volume", json_object_new_int(p_songNode->volume));
		json_object_object_add(p_data, "current", json_object_new_int(p_songNode->current));
		json_object_object_add(p_data, "total", json_object_new_int(p_songNode->total));
		json_object_object_add(p_data, "msgTime", json_object_new_string(p_songNode->msgTime));
		json_object_object_add(p_data, "picUrl", json_object_new_string(p_songNode->picUrl));
		json_object_object_add(p_data, "sourceType", json_object_new_int(1));
	}
	request_json_struct.data = p_data;

	request_json_struct.raw = NULL;
	//make json_data
	json_data = json_make_request_data(&request_json_struct);

	//check json_data
	if (NULL == json_data) {
		gatPrintf( GAT_DEBUG,"make json_data object failed.\n");
		return -1;
	}

	json_string = (char*)json_object_to_json_string(json_data);
	int dataLen = strlen(json_string);
	printf("%s:%d: %s\n ##### Len:%d \n",__func__,__LINE__,json_string,dataLen);
	ret = send_server_data(json_string, type);
	json_object_put(json_data);
	miguPrintf( GAT_DEBUG,"%s data send return %d\n", __func__,ret);
	return 0;
}

void mediaplayerEventHandle(mediaNode_t* pMediaPlayerNode)
{
	gatPrintf( GAT_DEBUG,"enter %s\n",__func__);
	if(pMediaPlayerNode == NULL)
	{
		miguPrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}
	//iofLock(&pMediaPlayerNode->songLock);
	if(pMediaPlayerNode->play_url ==NULL)
	{
		gatPrintf( GAT_DEBUG,"song_url is NULL\n");
		return ;	
	}
    songURL = pMediaPlayerNode->play_url;
	//iofUnlock(&pMediaPlayerNode->songLock);
	setDevStatus(DEV_STATUS_PLAYMEDIA, 1);
	gatPrintf( GAT_DEBUG,"play URL %s\n",songURL);
	return ;
}

static void all_pause_action(void) {
	printf("[push music]%s,%d,status:%d\n", __func__, __LINE__,getPlayType());
	if (getDevStatus()&DEV_STATUS_ALARM) {
		printf("[push music]%s,%d\n", __func__, __LINE__);
		//splayer_stop();
		setDevStatus(DEV_STATUS_ALARM, 0);
		sendInterruptEvt(TYPE_RESULT);
		led_all_off_mute();
	} else {
		setDevStatus(DEV_STATUS_PAUSE, 1);
		//splayer_stop();
	}
#if 0
	else if(getPlayType()==PLAYTYPE_MUSIC||getPlayType()==PLAYTYPE_BOOK||
				getPlayType()==PLAYTYPE_MEDIA||getPlayType()==PLAYTYPE_NEWS)  {
		printf("[push music]%s,%d\n", __func__, __LINE__);
		//if(getDevStatus()&DEV_STATUS_PLAYING) {
		//	printf("[push music]%s,%d\n", __func__, __LINE__);
		//	setDevStatus(DEV_STATUS_PAUSE, 1);
		//	play_set_pause(0);
		//}
		setDevStatus(DEV_STATUS_PAUSE, 1);
		splayer_stop();
	} else if(getPlayType()==PLAYTYPE_BLUETOOTH||getPlayType()==PLAYTYPE_TEXT||getPlayType()==PLAYTYPE_RESTAURANT
				||getPlayType()==PLAYTYPE_JOKE||getPlayType()==PLAYTYPE_RESTAURANT||getPlayType())
	{
		printf("[push music]%s,%d\n", __func__, __LINE__);
		bookNode_t *pbookNode =get_bookNode();
		if(pbookNode->status == DEV_STATUS_PLAYING) {
			printf("[push music]%s,%d\n", __func__, __LINE__);
			pbookNode->status = 0;
		}
		setDevStatus(DEV_STATUS_PAUSE, 1);
		splayer_stop();
	} else {
		printf("[push music]%s,%d\n", __func__, __LINE__);
		// retrurn -1 ??????
	}
#endif
}

void musicplayerEventHandle(songNode_t *pMediaPlayerNode)
{
	int ret=-1;
	if(pMediaPlayerNode == NULL)
	{
		miguPrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}

    if (pMediaPlayerNode->song_url == NULL) {
	gatPrintf( GAT_DEBUG,"song_url is NULL\n");
	return ;
    }

	if((getPlayType()==PLAYTYPE_BLUETOOTH)&&(getDevStatus()&DEV_STATUS_PLAYING)) {
		setDevStatus(DEV_STATUS_PLAYBLUETOOTH, 0);
		//btControlPause();
		gatPrintf( GAT_DEBUG,"func:%s,line:%d, stop bluetooth player\n",__func__,__LINE__);
	}

    songURL = pMediaPlayerNode->song_url;

	if(pMediaPlayerNode->play_right_now==1)
	{
		pMediaPlayerNode->play_right_now = 0;
		printf("### push music\n");	
		if (1 == _player_run_flag) 
			//play_release();
		all_pause_action();
		usleep(100*1000);	
		_player_run_flag = 1;
	}
	else
	{
		printf("### request music,padding mode\n");	
	}
	setDevStatus(DEV_STATUS_PLAYMUSIC, 1);
	ret = package_json_pushdevinfo_action();
	printf("%s:%d pushdevinfo return: %d\n", __func__,__LINE__,ret);
}

int play_textbook_handle(bookNode_t *pBookPlayerNode){
	int ret = 0;
	int line =0;
	char buffer[512]={0};

	if(pBookPlayerNode==NULL){
		printf("%s error para\n", __func__);
		return -1;
	}
	if(1){
		printf("%s debug\nstatus:%d\nchapterUrl:%s\ncontentName:%s\n"
			"chapterName:%s\nextId:%s\nprevId:%s\nchapterSize:%s\n"
			"playUrl:%s\n,isFree:%d\n",
			__func__,
			pBookPlayerNode->status,
			pBookPlayerNode->chapterUrl,
			pBookPlayerNode->contentName,
			pBookPlayerNode->chapterName,
			pBookPlayerNode->nextId,
			pBookPlayerNode->prevId,
			pBookPlayerNode->chapterSize,
			pBookPlayerNode->payUrl,
			pBookPlayerNode->isFree);
	}

	//step1 check status
	if( isCloudPhoneBusy()||isBtPhoneBusy() ||  (getDevStatus() & DEV_STATUS_ALARM))
	{
		gatPrintf( GAT_DEBUG,"func:%s,line:%d, phone busy or alarm status\n",__func__,__LINE__);
		return 0;
	}

	//step2 change status
	if(getDevStatus()&DEV_STATUS_PLAYING&&(getPlayType()==PLAYTYPE_MUSIC||(getPlayType()==PLAYTYPE_BOOK)||
		(getPlayType()==PLAYTYPE_MEDIA)||(getPlayType()==PLAYTYPE_NEWS))) 
	{
		//play_set_pause(0);
		//play_release();
		//usleep(100*1000);
		gatPrintf( GAT_DEBUG,"func:%s,line:%d, stop gstplayer\n",__func__,__LINE__);
	}
	else if(getDevStatus()&DEV_STATUS_PLAYING&&(getPlayType()==DEV_STATUS_PLAYBLUETOOTH)) 
	{
		//btControlStop();
		gatPrintf( GAT_DEBUG,"func:%s,line:%d, stop bluetooth player\n",__func__,__LINE__);
	}

	//check for pay
	if(strlen(pBookPlayerNode->payUrl)>6)
	{
		setDevStatus(DEV_STATUS_PLAYTEXT,0);
		setDevStatus(PLAYTYPE_BOOK,0);
		setDevStatus(DEV_STATUS_PLAYING,0);
		sprintf(pBookPlayerNode->curLine,"%s%s",pBookPlayerNode->chapterName, "。本章节是付费内容，请先使用手机客户端付费");
		playermanager_play_text(pBookPlayerNode->curLine);
		return 0;
	}

	//step  release old url, open new url
	if(pBookPlayerNode->handle){
		url_fclose(pBookPlayerNode->handle);
		pBookPlayerNode->handle =NULL;
	}
	pBookPlayerNode->handle = url_fopen(pBookPlayerNode->chapterUrl, "r");
	if(!pBookPlayerNode->handle) {
		gatPrintf( GAT_DEBUG,"url_fopen() couldn't open url:%s\n", pBookPlayerNode->chapterUrl);
		setDevStatus(DEV_STATUS_PLAYTEXT,0);
		setDevStatus(DEV_STATUS_PLAYING,0);
		return -1;
	}

	//step2 change status
	setDevStatus(DEV_STATUS_PLAYTEXT,1);
	setDevStatus(DEV_STATUS_PLAYING,1);
	pBookPlayerNode->status = DEV_STATUS_PLAYING;

	//step3 begain play textbook
	sprintf(pBookPlayerNode->curLine,"%s%s",pBookPlayerNode->announce, pBookPlayerNode->contentName);
	pBookPlayerNode->announce[0]=0;
	playermanager_play_text(pBookPlayerNode->curLine);	
	return 0;
}

void bookplayerEventHandle(bookNode_t *pBookPlayerNode)
{
	int ret=-1;
    
	if(pBookPlayerNode == NULL)
	{
		miguPrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}
    
	int32 evt;
	int32 type = SOURCE_TYPE_GST_PLAY;	

	//evt = EVT_PLAYER_SWITCH;
	//statusEventNotify(evt, &type);

	aiMessage_t* p_aiMessage = &aiMessage;

	if ((pBookPlayerNode->isFree ==1) &&  strlen(pBookPlayerNode->payUrl) > 6)
	{
		songURL = pBookPlayerNode->payUrl;
	}
	else if(strlen(pBookPlayerNode->chapterUrl) > 6)
	{
		songURL = pBookPlayerNode->chapterUrl;
	}
	
	int contentName_len = strlen(pBookPlayerNode->contentName);
	int chapterName_len = strlen(pBookPlayerNode->chapterName);
	int total_len = contentName_len +chapterName_len + strlen(pBookPlayerNode->announce)+5;
	char * say_text = (char*)iofMalloc(total_len);
	if (strlen(pBookPlayerNode->announce) > 2)
	{
		if (say_text == NULL)
		{
			printf("%s:%d malloc say_test retrun NULL\n",__func__,__LINE__);
			if (1 == _player_run_flag)
				//play_release();
			return;
		}
		sprintf(say_text,"%s%s的%s",pBookPlayerNode->announce, 
									pBookPlayerNode->contentName,
									pBookPlayerNode->chapterName);
		// push ai message
		memset(p_aiMessage->answerText,0,256);
		sprintf(p_aiMessage->answerText,"%s",say_text);

		if (strlen(pBookPlayerNode->payUrl)==0)
		{
			ret = package_json_pushaiMessage_action();
			printf("%s:%d push aiMessage return: %d\n", __func__,__LINE__,ret);
		}

		memset(pBookPlayerNode->announce,0,32);
	}
	else
	{
		sprintf(say_text,"%s",pBookPlayerNode->chapterName);
	}

	setDevStatus(DEV_STATUS_PLAYBOOK, 1);
	playermanager_play_text(say_text);

	iofFree(say_text);
  
}

static int32 clockTimerCb( gatTimer_st *pTimer ) {
	char content[128]={0};
	int32 type;

	printf("%s\n", __func__);
	if(isCloudPhoneBusy()||isBtPhoneBusy())
	{
		gatPrintf( GAT_DEBUG,"telephone busy return\n");
		return 0;
	}
	strcpy(content, "主人，");
	if(pTimer->param && strlen(pTimer->param)<128){
		strcat(content, pTimer->param);
	}
	else{
		strcat(content, "闹钟");
	}
	strcat(content, "时间到了。");
	miguPrintf( GAT_DEBUG,"\r\nring ring ring:%s\n",content);


	if (getDevStatus() & DEV_STATUS_MOBAIHE_MODE)	
	{
		//MoBaiHe_exit();
#if 0
		getCaeUserData()->wakeupstatus = 1;
		caeWakeupEnable(1);
		setDevStatus(DEV_STATUS_MOBAIHE_MODE, 0);
		gatPrintf( GAT_DEBUG,"MoBaiHe End\n");
#endif
	}

	type = TYPE_NOTIFY;
	sendInterruptEvt(type);
	
	if (getDevStatus() & DEV_STATUS_ALARM)
	{
		gatPrintf( GAT_DEBUG,"reminder is playing, cancel play clock.\n");
	}
	else
	{
		playermanager_play_text(content);
		setDevStatus(DEV_STATUS_ALARM, 1);
	}

	return 0;
}

static int32 reminderTimerCb( gatTimer_st *pTimer )
{
	char content[128]={0};
	int32 type;

	printf("%s\n", __func__);
	if(isCloudPhoneBusy()||isBtPhoneBusy())
	{
		gatPrintf( GAT_DEBUG,"telephone busy return\n");
		return 0;
	}

	strcpy(content, "主人，");
	if(pTimer->param && strlen(pTimer->param)<128){
		strcat(content, pTimer->param);
	}
	else{
		strcat(content, "提醒");
	}
	strcat(content, "时间到了。");
	miguPrintf( GAT_DEBUG,"\r\nring ring ring:%s\n",content);

	setDevStatus(DEV_STATUS_ALARM, 1);
	if (getDevStatus() & DEV_STATUS_MOBAIHE_MODE)	
	{
#if 0
		getCaeUserData()->wakeupstatus = 1;
		caeWakeupEnable(1);
		setDevStatus(DEV_STATUS_MOBAIHE_MODE, 0);
		gatPrintf( GAT_DEBUG,"MoBaiHe End\n");
#endif
		//MoBaiHe_exit();
	}

	type = TYPE_NOTIFY;
	sendInterruptEvt(type);

	playermanager_play_text(content);

	return 0;
}

int update_timer_to_file(timerNode_t* ptimerNode,int is_from_file)
{
#define TIMER_FILE "/data/timer.json"
	json_object * timer_root;
	json_object * clock;
	json_object * reminder;
	int ret;

	// no old file.
	if(access(TIMER_FILE, F_OK)){//should move to /data
		gatPrintf( GAT_DEBUG,"%s: create new timer_root\n", __func__);
		timer_root = json_object_new_object();
	}
	//have old file.
	else
	{
		timer_root = json_object_from_file(TIMER_FILE);
		if (timer_root == NULL)
		{
			timer_root = json_object_new_object();
		}
	}
	//printf("%s: \n%s\n",__func__,json_object_to_json_string_ext(timer_root,2));

	if (!strcmp(ptimerNode->intent, "setupClock")) {
		ret = json_pointer_get(timer_root,"clock", &clock);
		if (ret == 0){
			json_object_object_del(timer_root , "clock");
			json_object_put(clock);
		}
		clock = json_object_new_object();
		if (clock == NULL)
		{
			return -1;
		}
		if (ptimerNode->timerSetupTime != NULL)
			json_object_object_add(clock, "timerSetupTime", json_object_new_string(ptimerNode->timerSetupTime));
		
		json_object_object_add(timer_root , "clock", clock);
	}
	else if (!strcmp(ptimerNode->intent, "setupReminder")) {
		ret = json_pointer_get(timer_root,"reminder", &reminder);
		if (ret == 0){
			json_object_object_del(timer_root , "reminder");
			json_object_put(reminder);
		}

		reminder = json_object_new_object();
		if (reminder == NULL)
		{
			return -1;
		}
		if (ptimerNode->timerSetupTime != NULL)
			json_object_object_add(reminder, "timerSetupTime", json_object_new_string(ptimerNode->timerSetupTime));
		if (ptimerNode->timer_name!= NULL)
			json_object_object_add(reminder, "timer_name", json_object_new_string(ptimerNode->timer_name));

		json_object_object_add(timer_root , "reminder", reminder);
	}
	else if (!strcmp(ptimerNode->intent, "cancel")) {
		if (!strcmp(ptimerNode->timer_name, "reminder")) {

			ret = json_pointer_get(timer_root,"/reminder", &reminder);
			if (ret && is_from_file == 0){
				playermanager_play_text("主人，没有发现可删除的提醒呢。");
				alarm_OK = 0;
			}
			json_object_object_del(timer_root , "reminder");
		}
		else if(!strcmp(ptimerNode->timer_name, "clock")) {
			ret = json_pointer_get(timer_root,"/clock", &clock);
			if (ret && is_from_file == 0){
				playermanager_play_text("主人，没有发现可删除的闹钟呢。");
				alarm_OK = 0;
			}
			json_object_object_del(timer_root , "clock");
		}
		else
		{
			gatPrintf( GAT_DEBUG,"intent cancel failed!\n");
		}
	}

	ret = json_object_to_file_ext(TIMER_FILE,timer_root ,2);
	printf("\n%s\n",json_object_to_json_string_ext(timer_root,2));

}

int update_timer_from_file()
{
	json_object * timer_root;
	json_object * clock;
	json_object * reminder;
	char *timerSetupTime;
	char *timer_name;
	int ret;

	printf("enter %s\n",__func__);
	timerNode_t* p_timerNode = get_timerData();


	if(access(TIMER_FILE, F_OK)){
		gatPrintf( GAT_DEBUG,"%s: no old timer.\n", __func__);
		return 0;
	}
	timer_root = json_object_from_file(TIMER_FILE);
	if (timer_root == NULL)
	{
		gatPrintf( GAT_DEBUG,"%s: no old timer.\n", __func__);
		return 0;
	}
	ret = json_pointer_get(timer_root,"/reminder", &reminder);
	if (ret==0){
		gatPrintf( GAT_DEBUG,"%s: have old reminder.\n", __func__);
		timerSetupTime = get_string_by_path(timer_root,"/reminder/timerSetupTime");
		timer_name = get_string_by_path(timer_root,"/reminder/timer_name");
		if (timerSetupTime != NULL && timer_name != NULL)
		{
			iofLock(&p_timerNode->timerLock);
			p_timerNode->timerSetupTime = timerSetupTime ;
			p_timerNode->timer_name = timer_name;
			memset(p_timerNode->intent,0,50);
			sprintf(p_timerNode->intent, "%s", "setupReminder");
			iofUnlock(&p_timerNode->timerLock);
			timerEventHandle(p_timerNode,1);
			//statusEventNotify(EVT_TIMER_ACTION, (timerNode_t*)&p_timerNode);
		}
	}
	else
	{
		gatPrintf( GAT_DEBUG,"%s: no old reminder.\n", __func__);
	}
	ret = json_pointer_get(timer_root,"/clock", &clock);
	if (ret== 0){
		gatPrintf( GAT_DEBUG,"%s: have old clock.\n", __func__);
		timerSetupTime = get_string_by_path(timer_root,"/clock/timerSetupTime");
		if (timerSetupTime != NULL)
		{
			iofLock(&p_timerNode->timerLock);
			p_timerNode->timerSetupTime = timerSetupTime ;
			p_timerNode->timer_name = NULL;
			memset(p_timerNode->intent,0,50);
			sprintf(p_timerNode->intent, "%s", "setupClock");
			iofUnlock(&p_timerNode->timerLock);
			timerEventHandle(p_timerNode,1);
			//statusEventNotify(EVT_TIMER_ACTION, (timerNode_t*)&p_timerNode);
		}
	}
	else
	{
		gatPrintf( GAT_DEBUG,"%s: no old clock.\n", __func__);
	}

	return 0;
}

void timerEventHandle(timerNode_t* ptimerNode, int is_from_file) 
{
	int i = 0;
	int j = 0;
	struct tm set_alram_date;
	struct tm* timer = &set_alram_date;
	char tmpBuf[15];
	void* state;
	int m_year, m_mounth, m_day, m_hour, m_min, m_sec;
	time_t alarm_time;
	time_t curent_time;
	int alarm_left_time = 0;
	

	if(ptimerNode == NULL)
	{
		acePrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}

	iofLock(&ptimerNode->timerLock);
	if (!strcmp(ptimerNode->intent, "cancel")) {
		printf("############# cancel timer ##############\n");
		if (!strcmp(ptimerNode->timer_name, "reminder")) {
			gatPrintf( GAT_DEBUG,"intent cancel reminder sucess!\n");
			gatTimerDel( &reminderTimer);
			update_timer_to_file(ptimerNode,is_from_file );
		}
		else if (!strcmp(ptimerNode->timer_name, "clock")) {
			gatPrintf( GAT_DEBUG,"intent cancel clock sucess!\n");
			gatTimerDel( &clockTimer);
			update_timer_to_file(ptimerNode,is_from_file );
		}
		else 
			printf("[err] intent cancel reminder/clock fail!\n");

		iofUnlock(&ptimerNode->timerLock);
		return;
	}
    
	if (ptimerNode->timerSetupTime == NULL) 
	{
		gatPrintf( GAT_DEBUG,"timerSetupTime is NULL\n");
		iofUnlock(&ptimerNode->timerLock);
		return ;
    }

	for (i=0; i<strlen(ptimerNode->timerSetupTime); i++) {
		if(ptimerNode->timerSetupTime[i]>= '0' && 
				ptimerNode->timerSetupTime[i] <= '9')
		tmpBuf[j++] = ptimerNode->timerSetupTime[i] - '0';
	}
	m_year = tmpBuf[0]*1000+tmpBuf[1]*100+tmpBuf[2]*10+tmpBuf[3];
	m_mounth = tmpBuf[4]*10+tmpBuf[5];
	m_day = tmpBuf[6]*10+tmpBuf[7]; 
	m_hour = tmpBuf[8]*10+tmpBuf[9]; 
	m_min = tmpBuf[10]*10+tmpBuf[11]; 
	m_sec = tmpBuf[12]*10+tmpBuf[13];
	
	set_alram_date.tm_year = m_year;
	set_alram_date.tm_mon  = m_mounth;
	set_alram_date.tm_mday = m_day;
	set_alram_date.tm_hour = m_hour;
	set_alram_date.tm_min  = m_min;
	set_alram_date.tm_sec  = m_sec;

	printf("############# setup timer ##############\n");
	printf("######### %d-%02d-%02d %02d:%02d:%02d ##########\n", 
			timer->tm_year, timer->tm_mon, timer->tm_mday,
			timer->tm_hour, timer->tm_min, timer->tm_sec);
	printf("########################################\n");

	timer->tm_year -=1900;
	timer->tm_mon -=1;
	alarm_time = mktime(timer);
	curent_time = time(NULL);
	alarm_left_time = alarm_time - curent_time;
	if (alarm_left_time < 0) {
		gatPrintf( GAT_DEBUG,"Set time is earlier than actual time, please set again!!!\n");
		gatPrintf( GAT_DEBUG,"is_from_file = %d\n",is_from_file);
		if (is_from_file == 0)
		{
			playermanager_play_text("这个难度有点高，我还不知如何设置过去的闹钟");
			alarm_OK = 0;
		}
#if 0
		if (!strcmp(ptimerNode->intent, "setupClock")) {
			ptimerNode->timer_name = "clock"; 
		}
		if (!strcmp(ptimerNode->intent, "setupReminder")) {
			ptimerNode->timer_name = "reminder"; 
		}
		memset( ptimerNode->intent,0,50);
		sprintf(ptimerNode->intent, "%s", "cancel");
		printf("intent:%s, timer_name:%s\n",ptimerNode->intent,ptimerNode->timer_name);
		update_timer_to_file(ptimerNode,is_from_file );
#endif
		iofUnlock(&ptimerNode->timerLock);
		return NULL;
	}
	miguPrintf( GAT_DEBUG,"alarm_left_time:%d!!!\n", alarm_left_time);

	if (!strcmp(ptimerNode->intent, "setupClock")) {
		gatPrintf( GAT_DEBUG,"get event setupClock.\n");
		gatTimerAdd( &clockTimer, "clockTimer",alarm_left_time,clockTimerCb,ptimerNode->timer_name);
		update_timer_to_file(ptimerNode,is_from_file );
	}
	else if (!strcmp(ptimerNode->intent, "setupReminder")) {
		gatPrintf( GAT_DEBUG,"get event setupReminder.\n");
		gatTimerAdd( &reminderTimer, "reminderTimer",alarm_left_time,reminderTimerCb,ptimerNode->timer_name);
		update_timer_to_file(ptimerNode,is_from_file );
	}
	else {
		gatPrintf( GAT_DEBUG,"get event NULL.\n");
	}
	iofUnlock(&ptimerNode->timerLock);
	return ;
}

void *mediaplayerThread(void *arg)
{

    sigset_t signal_mask;
    
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) == -1) {
      perror("SIGPIPE");
    }

    while(1) {
		if (1 == _player_run_flag) {
			if (getDevStatus() & DEV_STATUS_PLAYBLUETOOTH)
			{
				_player_run_flag = 0;
				continue;
			}

			//splayer_stop();
			//btControlStop();
			//gst_play_action(songURL);
			_player_run_flag = 0;
			setDevStatus(DEV_STATUS_PLAYING, 1);
    	} 
		else usleep(100*1000);
    }
}



void tcpEventHandle(tcpSendListNode_t* clientSndNode)
{
	int32 ret = GAT_OK;
	int32 fd = 0;
	int32 type = 0;
	int32 beatCnt = 0; /*>=3: close socket, others:send data */

	if(clientSndNode == NULL)
	{
		gatPrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}

	fd = clientSndNode->fd;
	type = (int8)clientSndNode->arg;
    gatPrintf(GAT_DEBUG,"%s fd: %d, type:%d \r\n",__FUNCTION__, fd, type);
	
	if( type == MIGU_HEARTBEAT)
	{
		beatCnt = checkBeatCnt();
		if(beatCnt >= 3)
		{
			gatCloseTcpSocket(fd);
			setClientStep(1);
			setClientFd(INVALID_SOCKET);
			gatTimerDel(getHeartbeatTimer());
			gatTimerDel(getRequestHandleTimer());
			gatTimerDel(getUpgradeInfoTimer());
			return;
		}
	}
	
	ret = tcpSendHandle(fd);

	//if((type == MIGU_SEMANTIC|| type == MIGU_PUSHDEVINFO || type == MIGU_BOOKINFO) && ret == GAT_OK)
	if((type == MIGU_SEMANTIC) && ret == GAT_OK)
	{
		miguPrintf(GAT_DEBUG,"%s, add request timeout timer!  \r\n",__FUNCTION__);
		gatTimerAdd(getRequestHandleTimer(), "requestHandleTimer",10,requestHandleTimerCb,NULL);
	}
	miguPrintf(GAT_DEBUG,"%s, go out.  \r\n",__FUNCTION__);
	return;    
}

static void miguClientNodeInit(miguClientNode_t *node)
{
    node->fd = INVALID_SOCKET;
    node->port = MIGU_SERVER_PORT;
    node->sendBuf = NULL;
    node->sendLen = 0;
    node->arg = NULL;
    node->reconnTimes = 0;
	node->step = 1;
    iofMemset(node->szDomain,0, DOMAIN_LEN_MAX);
    iofMemset(node->szIp,0, IP_LEN_MAX);
    iofLockInit(&node->cliLock, KEYID_CLIENT_SYNC);
	#if USING_TEST_IP
    node->port = 48888;
    if(1)//(iofStrlen(node->szIp) != 0)
   	{
		memcpy(node->szIp, "112.13.96.203", strlen("112.13.96.203"));
    }	
	#endif
	memcpy(node->szDomain, MIGU_SERVER_DOMAIN, strlen(MIGU_SERVER_DOMAIN));
    miguPrintf( GAT_DEBUG,"szDomain:%s, Ip:%s, port:%d \r\n", node->szDomain, node->szIp, node->port);
}


int32 migu_client_init()
{
    int32 ret = GAT_ERR_FAIL;
	tcpSendInit();	
    clientNode = (miguClientNode_t *)iofMalloc(sizeof(miguClientNode_t));
    if(NULL == clientNode)
    {
        miguPrintf( GAT_ERROR,"no enough mem for malloc\r\n");
        return GAT_ERR_NORES;
    }
    miguClientNodeInit(clientNode);

	return GAT_OK;
}


int32 home_server_init()
{
    int32 ret = GAT_ERR_FAIL;
	char * migu_device_info=NULL;
	char migu_sdk_version[32]={0};

	gatPrintf( GAT_DEBUG,"enter %s\n",__func__);

	migu_device_info = json_make_migu_devinfo();
	if(migu_device_info == NULL)
	{
		miguPrintf( GAT_ERROR,"json_make_migu_devinfo failed!\r\n");
		return GAT_ERR_FAIL;
	}
	printf("%s:%d migu_device_info:%s\n",__func__,__LINE__,migu_device_info);
	ret = migusdk_init(migu_device_info);
	if(ret < 0)
	{
		miguPrintf( GAT_ERROR,"migusdk_init failed!\r\n");
		return GAT_ERR_FAIL;
	}
	
	getSdkVersion(migu_sdk_version);
	gatPrintf( GAT_DEBUG,"migu_sdk_version:%s\n",migu_sdk_version);


	iofFree(migu_device_info);

	migu_play_list_init();

	printf("%s OK, return\n",__func__);
	return GAT_OK;
}

int dnsByAddr(int8 *domain, uint8 *strIp)
{
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    int ret=-1;
    struct sockaddr_in *addr;
    char ipbuf[16];

	gatPrintf( GAT_DEBUG,"enter %s\n",__func__);
    if(domain == NULL)
	{
		miguPrintf( GAT_ERROR,"domain is NULL!\r\n");
		return -1;
	}

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;
    
    ret = getaddrinfo(domain, NULL,&hints,&res);
	
	miguPrintf( GAT_ERROR," %s,getaddrinfo:ret:%d \r\n", __FUNCTION__,ret);
    if (ret < 0) 
	{
		miguPrintf( GAT_ERROR,"getaddrinfo error, reason:%s !\r\n",gai_strerror(ret));
        return -1;
    }
    
    for (cur = res; cur != NULL; cur = cur->ai_next) 
	{
        addr = (struct sockaddr_in *)cur->ai_addr;
        printf("%s\n", inet_ntop(AF_INET,&addr->sin_addr, ipbuf, 16));
    }
	sprintf(strIp,"%s",inet_ntop(AF_INET,&addr->sin_addr, ipbuf, 16));
	miguPrintf( GAT_DEBUG,"ip :%s \r\n", strIp);
    freeaddrinfo(res);
	printf("%s OK, return\n",__func__);
	return 0;
}

int32 home_server_connect()
{
    int32 ret = GAT_ERR_FAIL;
	int32 fd=INVALID_SOCKET;
	gatPrintf( GAT_DEBUG,"enter %s\n",__func__);
	//iofLock(&getMiguClientNode()->cliLock);
	#if USING_TEST_IP
	miguPrintf( GAT_ERROR,"using test ip, ip:%s!\r\n", clientNode->szIp);
	#else
	ret = gatIofGetHostByName(clientNode->szDomain, clientNode->szIp);
	if(ret < 0 || iofStrlen(clientNode->szIp)==0)
	{
		ret = dnsByAddr(clientNode->szDomain, clientNode->szIp);
		if(ret < 0)
		{
			miguPrintf( GAT_ERROR,"dns failed, domain:%s!\r\n", clientNode->szDomain);
			//iofUnlock(&getMiguClientNode()->cliLock);
			return GAT_ERR_FAIL;
		}
	}
	#endif
    clientNode->fd = gatIofSocketOpen();
    if(clientNode->fd < 0)
	{
		miguPrintf( GAT_ERROR,"socket failed, fd:%d!\r\n", clientNode->fd);
		//iofUnlock(&getMiguClientNode()->cliLock);
		return GAT_ERR_FAIL;
	}
	miguPrintf( GAT_NOTIC,"fd:%d,ip:%s,port:%d!\r\n", clientNode->fd, clientNode->szIp, clientNode->port);
    ret = gatIofSocketConnect(clientNode->fd, clientNode->szIp, clientNode->port,NULL);
    if(ret < 0)
	{
		if(clientNode->fd >= 0)
		{
			gatIofSocketClose(clientNode->fd);
			clientNode->fd = INVALID_SOCKET;
		}
		miguPrintf( GAT_ERROR,"connect failed, ret:%d!\r\n", ret);
		//iofUnlock(&getMiguClientNode()->cliLock);
		return GAT_ERR_FAIL;
	}
	miguPrintf( GAT_DEBUG,"connect success, fd:%d!\r\n", clientNode->fd);
	fd=clientNode->fd;
	//iofUnlock(&getMiguClientNode()->cliLock);
	
	printf("%s OK, return\n",__func__);
	return fd;
}

/*
int miguTbook(int8* data, int32 len)
{
	int32 fd = -1;
	int32 ret = GAT_OK;
	int32 dataLen = 0;
	int32 totalLen = 0;
	uint8 header[MIGU_HEADER_LEN]={0};
	uint8* totalData=NULL;
	int32 type=MIGU_COMMUNICATION;

	if(NULL == clientNode)
	{
		tcpPrintf( GAT_ERROR,"client socket hasn't created!\r\n");
		return GAT_ERR_FAIL;
	}
	
	fd = clientNode->fd;    	
	dataLen = iofStrlen(data);
	header[0]=(unsigned char )(dataLen >> 24)&0xFF;
	header[1]=(unsigned char )(dataLen >> 16)&0xFF;
	header[2]=(unsigned char )(dataLen >> 8)&0xFF;
	header[3]=(unsigned char )(dataLen >> 0)&0xFF;
	totalLen = dataLen+MIGU_HEADER_LEN;
	totalData= (uint8*)iofMalloc(totalLen);
    	if(NULL == totalData)
    	{
        	tcpPrintf( GAT_ERROR,"malloc totalData buf fail\r\n");
        	return GAT_ERR_FAIL;
    	}
	
    	iofMemset(totalData,0, totalLen);
    	iofMemcpy(totalData, header, MIGU_HEADER_LEN);
    	iofMemcpy(totalData+MIGU_HEADER_LEN, data, dataLen);
    	ret = gatTcpSend(fd, totalData,totalLen,(int32*)type);
	if(ret != GAT_OK)
	{
		miguPrintf( GAT_ERROR,"gatTcpSend failed.\n");
		return GAT_ERR_FAIL;
	}
	iofFree(totalData);
	totalData = NULL;
	return ret;
}
*/
int send_server_data(char * data, int32 type)
{
	int32 fd = -1;
	int32 ret = GAT_OK;
	int32 dataLen = 0;
	int32 totalLen = 0;
	uint8 header[MIGU_HEADER_LEN]={0};
	uint8* totalData=NULL;

	if(NULL == clientNode)
	{
		tcpPrintf( GAT_ERROR,"client socket hasn't created!\n");
		return GAT_ERR_FAIL;
	}
	
	fd = clientNode->fd;    	
	if(fd < 0)
	{
		miguPrintf( GAT_ERROR,"migu client disconnect from server, fd:%d!\n", fd);
		return GAT_ERR_FAIL;
	}
	
	if(data==NULL)
	{
		miguPrintf(GAT_ERROR,"data invalid\n");
		return GAT_ERR_FAIL;
	}

	dataLen = iofStrlen(data);
	header[0]=(unsigned char )(dataLen >> 24)&0xFF;
	header[1]=(unsigned char )(dataLen >> 16)&0xFF;
	header[2]=(unsigned char )(dataLen >> 8)&0xFF;
	header[3]=(unsigned char )(dataLen >> 0)&0xFF;
	totalLen = dataLen+MIGU_HEADER_LEN;
	totalData= (uint8*)iofMalloc(totalLen);

    if(NULL == totalData)
    {
    	tcpPrintf( GAT_ERROR,"malloc totalData buf fail\n");
    	return GAT_ERR_FAIL;
    }
    iofMemset(totalData,0, totalLen);
    iofMemcpy(totalData, header, MIGU_HEADER_LEN);
	iofMemcpy(totalData+MIGU_HEADER_LEN, data, dataLen);
    ret = gatTcpSend(fd, totalData,totalLen,(int32*)type);
	if(ret != GAT_OK)
	{
		miguPrintf( GAT_ERROR,"gatTcpSend failed.\n");
		return GAT_ERR_FAIL;
	}
	iofFree(totalData);
	totalData = NULL;
	return ret;
}

int miguSemantic(int8* data, int32 len)
{
	int32 fd = -1;
	int32 ret = GAT_OK;
	int8* pData = NULL;
	int32 dataLen = 0;
	int32 totalLen = 0;
	uint8 header[MIGU_HEADER_LEN]={0};
	uint8* totalData=NULL;
	int32 type=MIGU_SEMANTIC;
	
	struct json_object* json_data;
	struct RequestJsonStruct request_json_struct;

	if(NULL == clientNode)
	{
		tcpPrintf( GAT_ERROR,"client socket hasn't created!\r\n");
		return GAT_ERR_FAIL;
	}

	fd = clientNode->fd;
	if(fd < 0)
	{
		miguPrintf( GAT_ERROR,"migu client disconnect from server, fd:%d!\r\n", fd);
		return GAT_ERR_FAIL;
	}
	
	if(data==NULL || data<=0)
	{
		miguPrintf( GAT_ERROR,"data invalid, dataLen:%d!\r\n", len);
		return GAT_ERR_FAIL;
	}
	memset(&request_json_struct,0,sizeof(struct RequestJsonStruct));
	
	//miguPrintf( GAT_DEBUG,"fun:%s, line:%d!\r\n", __FUNCTION__, __LINE__);
	/*make Semantic str*/
	//put msgType
	request_json_struct.msgType = msgType[SEMANTIC];
	//put data
	request_json_struct.data =NULL;
	//put raw
	request_json_struct.raw = NULL;
	ret = json_make_asrText(&request_json_struct, data);
	if(ret != GAT_OK)
	{
		if (request_json_struct.data != NULL)
		{
			json_object_put(request_json_struct.data);
			request_json_struct.data =NULL;
		}
		
		if (request_json_struct.raw != NULL)
		{
			json_object_put(request_json_struct.raw);
			request_json_struct.raw = NULL;
		}
		miguPrintf( GAT_ERROR,"make asrText object failed.\n");
		return GAT_ERR_FAIL;
	}
	
	//make json_data
	json_data = json_make_request_data(&request_json_struct);
	//check json_data
	if (NULL == json_data) 
	{
		if (request_json_struct.data != NULL)
		{
			json_object_put(request_json_struct.data);
			request_json_struct.data =NULL;
		}
		
		if (request_json_struct.raw != NULL)
		{
			json_object_put(request_json_struct.raw);
			request_json_struct.raw = NULL;
		}
		miguPrintf( GAT_ERROR,"make json_data object failed.\n");
		return GAT_ERR_FAIL;
	}
	pData = (int8*)json_object_to_json_string(json_data);
	dataLen = iofStrlen(pData);
	miguPrintf( GAT_DEBUG,"Semantic dataLen:%d, pData:%s\n", dataLen, pData);
	
	header[0]=(unsigned char )(dataLen >> 24)&0xFF;
	header[1]=(unsigned char )(dataLen >> 16)&0xFF;
	header[2]=(unsigned char )(dataLen >> 8)&0xFF;
	header[3]=(unsigned char )(dataLen >> 0)&0xFF;
	totalLen = dataLen+MIGU_HEADER_LEN;
    totalData= (uint8*)iofMalloc(totalLen);
    if(NULL == totalData)
    {
        tcpPrintf( GAT_ERROR,"malloc totalData buf fail\r\n");
		json_object_put(json_data);
        return GAT_ERR_FAIL;
    }
	
    iofMemset(totalData,0, totalLen);
    iofMemcpy(totalData, header, MIGU_HEADER_LEN);
    iofMemcpy(totalData+MIGU_HEADER_LEN, pData, dataLen);
    ret = gatTcpSend(fd, totalData,totalLen,(int32*)type);
	if(ret != GAT_OK)
	{
		json_object_put(json_data);
		miguPrintf( GAT_ERROR,"gatTcpSend failed.\n");
		return GAT_ERR_FAIL;
	}
	json_object_put(json_data);
	iofFree(totalData);
	totalData = NULL;
	return ret;
}

int getImsPhoneListAction(int32 fd)
{
	struct json_object* json_data;
	int32 ret = GAT_OK;
	struct RequestJsonStruct request_json_struct;
	int8* pData = NULL;
	int32 totalLen = 0;
	int32 dataLen = 0;
	uint8* totalData=NULL;
	uint8 header[MIGU_HEADER_LEN]={0};
	int32 type=MIGU_COMMUNICATION;

	/*make login str*/
	//put msgType
	request_json_struct.msgType = msgType[GET_IMS_PHONE_LIST];

	//put data
	request_json_struct.data =NULL;

	//put raw
	request_json_struct.raw = NULL;

	//make json_data
	json_data = json_make_request_data(&request_json_struct);
	//check json_data
	if (NULL == json_data) {
		gatPrintf( GAT_DEBUG,"make json_data object failed.\n");
		return -1;
	}
	
	pData = (int8*)json_object_to_json_string(json_data);
		
	dataLen = iofStrlen(pData);
	header[0]=(unsigned char )(dataLen >> 24)&0xFF;
	header[1]=(unsigned char )(dataLen >> 16)&0xFF;
	header[2]=(unsigned char )(dataLen >> 8)&0xFF;
	header[3]=(unsigned char )(dataLen >> 0)&0xFF;
	totalLen = dataLen+MIGU_HEADER_LEN;
    totalData= (uint8*)iofMalloc(totalLen);
	
    if(NULL == totalData)
    {
        tcpPrintf( GAT_ERROR,"malloc totalData buf fail\r\n");
		json_object_put(json_data);
        return GAT_ERR_FAIL;
    }
    iofMemset(totalData,0, totalLen);
    iofMemcpy(totalData, header, MIGU_HEADER_LEN);
    iofMemcpy(totalData+MIGU_HEADER_LEN, pData, dataLen);
    ret = gatTcpSend(fd, totalData,totalLen, (int32*)type);
	if(ret != GAT_OK)
	{
		json_object_put(json_data);
		miguPrintf( GAT_ERROR,"gatTcpSend failed.\n");
		return GAT_ERR_FAIL;
	}
	json_object_put(json_data);
	iofFree(totalData);
	totalData = NULL;
	
	return ret;
}

int home_server_Login(int32 fd)
{
	int32 ret = GAT_OK;
	int8* pData = NULL;
	int32 dataLen = 0;
	//int32 totalLen = 0;
	uint8 header[MIGU_HEADER_LEN]={0};
	uint8* totalData=NULL;
	int32 type=MIGU_COMMUNICATION;
	
	struct json_object* json_data;
	struct RequestJsonStruct request_json_struct;
	
	memset(&request_json_struct,0,sizeof(struct RequestJsonStruct));
	/*make login str*/
	//put msgType
	request_json_struct.msgType = msgType[LOGIN];
	//put data
	request_json_struct.data =NULL;
	//put raw
	request_json_struct.raw = NULL;
	//make json_data
	json_data = json_make_request_data(&request_json_struct);

	//check json_data
	if (NULL == json_data) 
	{
		miguPrintf( GAT_ERROR,"make json_data object failed.\n");
		return GAT_ERR_FAIL;
	}
	pData = (int8*)json_object_to_json_string(json_data);
	dataLen = iofStrlen(pData);
	miguPrintf( GAT_ERROR,"login dataLen:%d, pData:%s\n", dataLen, pData);
	//ret = tcp_send_package(pData, dataLen,fd,type);
	ret = send_server_data(pData, type);
//	json_object_put(json_data);
//	iofFree(totalData);
//	totalData = NULL;
	miguPrintf( GAT_DEBUG,"login data send return %d\n", ret);
	return ret;
}

int miguUpgradeInfo(int32 fd)
{
	int32 ret = GAT_OK;
	int8* pData = NULL;
	int32 dataLen = 0;
	//int32 totalLen = 0;
	uint8 header[MIGU_HEADER_LEN]={0};
	uint8* totalData=NULL;
	int32 type=MIGU_COMMUNICATION;
	
	struct json_object* json_data;
	struct RequestJsonStruct request_json_struct;
	
	memset(&request_json_struct,0,sizeof(struct RequestJsonStruct));
	/*make login str*/
	//put msgType
	request_json_struct.msgType = msgType[GET_UPGRADE_INFO];
	//put data
	request_json_struct.data =NULL;
	//put raw
	request_json_struct.raw = NULL;
	//make json_data
	json_data = json_make_upgrade_data(&request_json_struct);

	//check json_data
	if (NULL == json_data) 
	{
		miguPrintf( GAT_ERROR,"make json_data object failed.\n");
		return GAT_ERR_FAIL;
	}
	pData = (int8*)json_object_to_json_string(json_data);
	dataLen = iofStrlen(pData);
	miguPrintf( GAT_ERROR,"login dataLen:%d, pData:%s\n", dataLen, pData);
	//ret = tcp_send_package(pData, dataLen,fd,type);
	ret = send_server_data(pData, type);
//	json_object_put(json_data);
//	iofFree(totalData);
//	totalData = NULL;
	miguPrintf( GAT_DEBUG,"login data send return %d\n", ret);
	return ret;
}


int miguHeartbeat(int32 fd)
{
	int32 ret = GAT_OK;
	int8* pData = NULL;
	int32 dataLen = 0;
	int32 totalLen = 0;
	uint8 header[MIGU_HEADER_LEN]={0};
	uint8* totalData=NULL;
	int32 type=MIGU_HEARTBEAT;
	
	struct json_object* json_data;
	struct RequestJsonStruct request_json_struct;
	
	memset(&request_json_struct,0,sizeof(struct RequestJsonStruct));
	/*make login str*/
	//put msgType
	request_json_struct.msgType = msgType[HEART_BEAT];
	//put data
	request_json_struct.data =json_tokener_parse("{\"beatVal\":\"ping\"},");
	//put raw
	request_json_struct.raw = NULL;
	//make json_data
	json_data = json_make_request_data(&request_json_struct);

	//check json_data
	if (NULL == json_data) 
	{
		miguPrintf( GAT_ERROR,"make json_data object failed.\n");
		return GAT_ERR_FAIL;
	}
	pData = (int8*)json_object_to_json_string(json_data);
	dataLen = iofStrlen(pData);
	miguPrintf( GAT_ERROR,"heartbeat dataLen:%d, pData:%s\n", dataLen, pData);
	
	header[0]=(unsigned char )(dataLen >> 24)&0xFF;
	header[1]=(unsigned char )(dataLen >> 16)&0xFF;
	header[2]=(unsigned char )(dataLen >> 8)&0xFF;
	header[3]=(unsigned char )(dataLen >> 0)&0xFF;
	totalLen = dataLen+MIGU_HEADER_LEN;
    totalData= (uint8*)iofMalloc(totalLen);
    if(NULL == totalData)
    {
        tcpPrintf( GAT_ERROR,"malloc totalData buf fail\r\n");
		json_object_put(json_data);
        return GAT_ERR_FAIL;
    }
	
    iofMemset(totalData,0, totalLen);
    iofMemcpy(totalData, header, MIGU_HEADER_LEN);
    iofMemcpy(totalData+MIGU_HEADER_LEN, pData, dataLen);
    ret = gatTcpSend(fd, totalData,totalLen,(int32*)type);

	if(ret != GAT_OK)
	{
		json_object_put(json_data);
		miguPrintf( GAT_ERROR,"gatTcpSend failed.\n");
		return GAT_ERR_FAIL;
	}
	json_object_put(json_data);
	iofFree(totalData);
	totalData = NULL;
	return ret;
}


void *homeserverThread(void *arg)
{
    int32 ret = GAT_OK;
    int32 fd = INVALID_SOCKET;
	static uint8 *tmpBuf = NULL;
	static uint8 *recBuf = NULL;
    gat_fd_set readfd;
    static int32 offset = 0;
	static int8 lenFlag = 1;
    static int32 waitTimeS=0;
    static int32 totalLen=0;
	static int32 bodyLen = 0;
	static int32 recLen = 0;
	static int32 wholePacketLen = 0;
	static int32 lastPacketBodyLen = 0;
	double elaVal = 0;
	double doubVal = 300.231;

    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) == -1)
    {
      perror("SIGPIPE");
    }

	ret = migu_client_init();
	if(ret != GAT_OK)
	{
		sleep(1);
		return NULL;
	}	
	ret = home_server_init();
	if( ret < 0)
	{
		setClientStep(3);
		sleep(1);
	}

	while(1)
	{
		if (getDevStatus() & DEV_STATUS_CONFIGWIFI)
		{
			sleep(1);
			continue;
		}
		if(getClientStep() == 3) //hasn't got mac
		{
			ret = home_server_init();
			if( ret < 0)
			{
				sleep(6);
				continue;
			}
			setClientStep(1);
		}
		#if 0 /*ping wlan,for future use */
		else if(initStep == 2) //hasn't connected to wlan
		{
			ret = netcheckHandle(&elaVal);
			miguPrintf( GAT_DEBUG,"%s elaVal:%.3lfms, doubVal:%.3lfms, ret:%d\r\n",__FUNCTION__,elaVal, doubVal, ret);
			if(elaVal > 300 || ret < 0)
			{
				sleep(8);
				continue;
			}
			initStep = 1;
			sleep(1);
		}
		#endif
		else if(getClientStep() == 1)
		{
			if(NULL == tmpBuf )
			{
				tmpBuf = (uint8 *)iofMalloc(RECBUF_LEN);
				if(NULL == tmpBuf )
				{
					miguPrintf( GAT_ERROR,"%s malloc size:%d fail\r\n",__FUNCTION__,RECBUF_LEN );
					break ;
				}
			}
			iofMemset(tmpBuf , 0, RECBUF_LEN);
			miguPrintf( GAT_DEBUG,"%s begin to connect to homeserver!\r\n",__FUNCTION__ );
			setDevStatus(DEV_STATUS_CONNECTED, 0);
			fd = getClientFd();
			miguPrintf( GAT_DEBUG,"%s client fd:%d!\r\n",__FUNCTION__ , fd);
			if(fd < 0)
			{
#if 0
				ret = home_server_init();
				if( ret < 0)
				{
					setClientStep(3);
					sleep(1);
					continue;
				}
#endif
				fd = home_server_connect();
				if(fd < 0)
				{
					sleep(3); 
					continue;
				}
			}
			ret = home_server_Login(fd);
			if(ret < 0)
			{
				miguPrintf( GAT_ERROR,"%s home_server_Login fail\r\n",__FUNCTION__ );
				//continue;
			}
			setClientStep(0);
			lenFlag = 1;			
			gatTimerAdd(getHeartbeatTimer(), "heartbeatTimer",MIGU_HEARTBEAT_FREQ,heartbeatTimerCb,(void*)(getMiguClientNode()->fd));
			continue;
		}
		else if( getClientStep() == 0 )
		{
			// for 4 length data.
			if(lenFlag == 1)
			{	
				offset = 0;
				totalLen = 0;
				GAT_FD_ZERO( &readfd);
				GAT_FD_SET(fd, &readfd); 
				//ret = gatIofSelect(fd+1, &readfd, NULL, NULL,0,1000000 );
				ret = gatIofSelect(fd+1, &readfd, NULL, NULL,1,0);
				if(ret <= 0)
				{
					continue;
				}
				
				if(GAT_FD_ISSET( fd,&readfd ))
				{
					// get 4 header data, and buffer header.
					recLen = gatIofSocketTcpRecv( fd, tmpBuf , RECBUF_LEN, 0 );
					if( recLen<=0 )
					{
						//iofMemset(tmpBuf , 0, RECBUF_LEN);
						miguPrintf( GAT_ERROR,"%s line:%d recLen(%d) errno:%d!\r\n",__FUNCTION__, __LINE__,recLen, errno );
						if(fd>=0)
						{
							gatIofSocketClose(fd);
						}
						setClientStep(1);//initStep = 2 -> 1
						setClientFd(INVALID_SOCKET);
						gatTimerDel(getHeartbeatTimer());
						gatTimerDel(getRequestHandleTimer());
						gatTimerDel(getUpgradeInfoTimer());
						//setPhoneFlag(2);
						sleep(1);
						continue;
					}
					// 4 length data get failed. network too bad. discard it.
					else if( recLen< 4){
						miguPrintf( GAT_ERROR,"%s line:%d recLen(%d) errno:%d!\r\n",__FUNCTION__, __LINE__,recLen, errno );
						lenFlag = 1;
						iofMemset(tmpBuf , 0, RECBUF_LEN);
						continue;
					}
					bodyLen = parsingLen(tmpBuf );
					miguPrintf( GAT_NOTIC,"%s recLen: %d,bodyLen:%d\r\n",__FUNCTION__,recLen,bodyLen);
					//dumpBuf( GAT_DEBUG,((char *)tmpBuf ),recLen);
					if(bodyLen <= 0 || bodyLen > (20*1024 -4))
					{
						lenFlag = 1;
						iofMemset(tmpBuf, 0, RECBUF_LEN);
						miguPrintf( GAT_ERROR,"%s bodyLen(%d) overflow\r\n",__FUNCTION__, bodyLen );
						continue;
					}
					lenFlag = 0;
						
					offset = recLen-4;
					totalLen = bodyLen;
					if(NULL != recBuf)
					{
						iofFree(recBuf);
						recBuf = NULL;
					}
					recBuf = (uint8*)iofMalloc(bodyLen);		
					if(NULL == recBuf)
					{
						miguPrintf( GAT_ERROR,"%s malloc size:%d fail\r\n",__FUNCTION__,RECBUF_LEN );
						iofFree(tmpBuf);
						tmpBuf= NULL;
						break ;
					}
					if(bodyLen <= offset)//more than one packet
					{
						wholePacketLen = bodyLen;
					}
					else //bodyLen > offset, not recevd totally
					{
						wholePacketLen = offset;
					}
					iofMemcpy(recBuf, tmpBuf+4, wholePacketLen);
					miguPrintf( GAT_NOTIC,"%s offset: %d,bodyLen:%d\r\n",__FUNCTION__,offset,bodyLen);
					//dumpBuf( GAT_DEBUG,((char *)recBuf),(recLen-4));
					//recBuf[recLen-4]='\0';
				}
			}
			else if(lenFlag == 2)
			{
				lenFlag = 1;
				if(bodyLen < offset)//more than twices packet,not handled,drop it
				{
					waitTimeS = 0;
					if(NULL != recBuf)
					{
						iofFree(recBuf);
						recBuf = NULL;
					}
					iofMemset(tmpBuf , 0, RECBUF_LEN);
					miguPrintf( GAT_ERROR,"%s,couldn't handled more than twices packets!\r\n",__FUNCTION__ );
					sleep(1);
					continue;
				}
				else //bodyLen >= offset, recved totally or not recevd totally
				{
					wholePacketLen = offset;
				}
				recBuf = (uint8*)iofMalloc(bodyLen);		
				if(NULL == recBuf)
				{
					miguPrintf( GAT_ERROR,"%s malloc size:%d fail\r\n",__FUNCTION__,RECBUF_LEN );
					iofFree(tmpBuf);
					tmpBuf= NULL;
					break ;
				}
				iofMemset(recBuf , 0, bodyLen);
				iofMemcpy(recBuf, tmpBuf+4+lastPacketBodyLen+4, wholePacketLen);//4+lastPacketBodyLen:last packet header+bodylen
			}
			//for data 
			while( offset < bodyLen) /*hasn't recvd a whole packet */
			{
				miguPrintf( GAT_ERROR,"%s waitTimeS:%d!\r\n",__FUNCTION__, waitTimeS);
				waitTimeS++;
				if(waitTimeS == 2)
				{
					usleep(100*1000);
				}
				else if(waitTimeS == 3)
				{
					usleep(100*1000);
				}
				else if(waitTimeS == 4)
				{
					usleep(200*1000);
				}
				else if(waitTimeS == 5)
				{
					usleep(200*1000);
				}
				else if( waitTimeS>5 )
				{
					lenFlag = 1;
					waitTimeS = 0;
					//iofMemset(recBuf, 0, RECBUF_LEN);
					if(NULL != recBuf)
					{
						iofFree(recBuf);
						recBuf = NULL;
					}
					iofMemset(tmpBuf , 0, RECBUF_LEN);
					miguPrintf( GAT_ERROR,"%s recv timeout!\r\n",__FUNCTION__ );
					setClientStep(1);//initStep = 2 -> 1
					setClientFd(INVALID_SOCKET);
					gatTimerDel(getHeartbeatTimer());
					gatTimerDel(getRequestHandleTimer());
					gatTimerDel(getUpgradeInfoTimer());
					//setPhoneFlag(2);
					sleep(1);
					continue;
				}
				GAT_FD_ZERO( &readfd);
				GAT_FD_SET(fd, &readfd); 
				//ret = gatIofSelect(fd+1, &readfd, NULL, NULL,0,1000000 );
				ret = gatIofSelect(fd+1, &readfd, NULL, NULL,1,0);
				if( ret>0 && GAT_FD_ISSET( fd,&readfd ) )
				{
					iofMemset(tmpBuf , 0, RECBUF_LEN);
					//recLen = gatIofSocketTcpRecv( fd, recBuf+offset, bodyLen-offset, 0 );/*hasn't handled more than one pakcet */
					recLen = gatIofSocketTcpRecv( fd, tmpBuf , RECBUF_LEN, 0 );
					if( recLen<0 )
					{
						if(NULL != recBuf)
						{
							iofFree(recBuf);
							recBuf = NULL;
						}
						miguPrintf( GAT_ERROR,"%s line:%d recLen(%d) errno:%d!\r\n",__FUNCTION__, __LINE__,recLen, errno );
						if(fd>=0)
						{
							gatIofSocketClose(fd);
						}
						setClientStep(1);//initStep = 2 -> 1
						setClientFd(INVALID_SOCKET);
						gatTimerDel(getHeartbeatTimer());
						gatTimerDel(getRequestHandleTimer());
						gatTimerDel(getUpgradeInfoTimer());
						//setPhoneFlag(2);
						sleep(1);
						continue;
					}
					if( recLen <= (bodyLen-offset))//recvd a whole packet already or hasn't recvd a whole packet
					{
						wholePacketLen = recLen;
					}
					else //retLen > bodyLen-offset, more than one packet,droped last tmpBuf data
					{
						wholePacketLen = (bodyLen-offset);
					}
					iofMemcpy(recBuf+offset, tmpBuf, wholePacketLen);
					offset += wholePacketLen;
					miguPrintf( GAT_NOTIC,"%s,line:%d,recLen:%d,offset:%d,bodyLen:%d\r\n",__FUNCTION__,__LINE__,recLen,offset,bodyLen);
				}
			}
			if(offset >= bodyLen && bodyLen!=0)
			{
				lenFlag = 1;
				waitTimeS = 0;
				miguPrintf( GAT_NOTIC,"%s recv a whole packet!,one packet total Len:%d\r\n",__FUNCTION__,bodyLen );
				//dumpBuf( GAT_DEBUG,((char *)recBuf),offset);
				json_parser(recBuf,bodyLen);
				if(NULL != recBuf)
				{
					iofFree(recBuf);
					recBuf = NULL;
				}
				if(offset > bodyLen)
				{
					lastPacketBodyLen = bodyLen;
					bodyLen = parsingLen(tmpBuf+4+lastPacketBodyLen);//+4:last packet header length, bodyLen:last packet bodyLen
					miguPrintf( GAT_NOTIC,"%s line:%d,recLen: %d,bodyLen:%d\r\n",__FUNCTION__,__LINE__,recLen,bodyLen);
					//dumpBuf( GAT_DEBUG,((char *)tmpBuf ),recLen);
					if(bodyLen <= 0 || bodyLen > (20*1024 -4))
					{
						lenFlag = 1;
						iofMemset(tmpBuf, 0, RECBUF_LEN);
						miguPrintf( GAT_ERROR,"%s bodyLen(%d) overflow\r\n",__FUNCTION__, bodyLen );
						continue;
					}
					lenFlag = 2;
					offset = (recLen- 4 - lastPacketBodyLen)-4;//figured out current temp buffer(tmpBuf) hav't handled data
					miguPrintf( GAT_NOTIC,"%s line:%d,offset: %d,bodyLen:%d\r\n",__FUNCTION__,__LINE__,offset,bodyLen);
				}
			}
		}
	}
	miguPrintf( GAT_NOTIC,"%s, thread exit\r\n",__FUNCTION__ );
	return NULL;
}

int32 AP_config_TimeoutTimerCb( gatTimer_st *pTimer )
{
	printf("enter %s\n",__func__);
	setDevStatus(DEV_STATUS_AP_CONFIG, 0);
	setDevStatus(DEV_STATUS_CONFIGWIFI, 0);
	//caeWakeupEnable(1);	
	playermanager_play_file("/media/sound/wificonfig_connect_fail.pcm");

	system("/usr/bin/ap_config.sh stop");
	sleep(2);
	system("/etc/init.d/S42wifi start");
	sleep(3);

	setClientStep(3);
	return 0;

}
int ap_config_handle()
{
	//playermanager_play_file("/media/sound/andlink_start.pcm");

	gatTimerDel(getHeartbeatTimer());
	gatTimerDel(getRequestHandleTimer());
	gatTimerDel(getUpgradeInfoTimer());

    setDevStatus(DEV_STATUS_AP_CONFIG, 1);
    setDevStatus(DEV_STATUS_CONFIGWIFI, 1);

	gatTimerAdd(getAPConfigTimeoutTimer(), "ap_config_TimeoutTimer",90,AP_config_TimeoutTimerCb,NULL);

	#if 0
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

	cloudphone_cmd = PHONE_DEREGIST;
	pthread_cond_signal(&cloudphone_condition);
	pthread_mutex_unlock(&cloudphone_mutex);
	#endif

	sleep(2);
	system("/etc/init.d/S42wifi stop");
	sleep(2);
	system("/usr/bin/ap_config.sh start");


	start_ap_config();
}
