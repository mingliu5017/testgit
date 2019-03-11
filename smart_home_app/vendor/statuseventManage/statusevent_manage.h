/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _STATUS_EVT_MANAGE_
#define _STATUS_EVT_MANAGE_

#ifdef __cplusplus
extern "C"{ 
#endif

#include "gatplatform.h"
#include "gattypes.h"
#include "gatlist.h"
#include "iofipc.h"
#include "gatgobal.h"
#include "fopen.h"

/*dev status */
#define DEV_STATUS_MICMUTE           (1<<0)
#define DEV_STATUS_MUTE_PENDING		 (1<<1)
#define DEV_STATUS_IVW_ACKING		 (1<<2)

// cloudphone
#define DEV_STATUS_CALLING           (1<<3)
#define DEV_STATUS_RINGING           (1<<4) /*有电话打入 */
#define DEV_STATUS_TALKING           (1<<5)
#define DEV_STATUS_HANGUP            (1<<6)
//bt phone
#define DEV_STATUS_BTPHONE_TALKING	 (1<<7)
#define DEV_STATUS_BTPHONE_RINGING 	 (1<<8)
#define DEV_STATUS_BTPHONE_CALLING 	 (1<<9)

//play status
#define DEV_STATUS_PLAYING           (1<<10)
#define DEV_STATUS_IDLE              (1<<11)
#define DEV_STATUS_PAUSE             (1<<12)
#define DEV_STATUS_STOP              (1<<13) /*stop to play caused by wakeup */
#define DEV_STATUS_PLAYBOOK          (1<<14)
#define DEV_STATUS_PLAYMUSIC         (1<<15)
#define DEV_STATUS_PLAYTEXT          (1<<16)
#define DEV_STATUS_ALARM          	 (1<<17)
#define DEV_STATUS_PLAYMEDIA		 (1<<18)
#define DEV_STATUS_PLAYRESTAURANT	 (1<<19)
#define DEV_STATUS_PLAYBLUETOOTH	 (1<<20)
#define DEV_STATUS_PLAYNEWS	         (1<<21)
#define DEV_STATUS_JOKE	             (1<<22)
#define DEV_STATUS_POETRY	         (1<<23)

//connction status
#define DEV_STATUS_CONNECTED         (1<<24)
#define DEV_STATUS_CONFIGWIFI        (1<<25)
#define DEV_STATUS_MOBAIHE_MODE	     (1<<26)
#define DEV_STATUS_MOBAIHE_SEARCH	 (1<<27)
#define DEV_STATUS_BT_CONNECTED		 (1<<28)
#define DEV_STATUS_AP_CONFIG		 (1<<29)


#define DEV_STATUS_MASK             0xFFFFFFFF
#define DEV_STATUS_CLEAR            0x0

/*speaker status */
#define SPEAKER_ACTION_IDLE              0
#define SPEAKER_ACTION_READY             1
#define SPEAKER_ACTION_PLAYING           2
#define SPEAKER_ACTION_PAUSE             3
#define SPEAKER_ACTION_STOP              4
#define SPEAKER_ACTION_INVALID           5

#define PLAYTYPE_MUSIC         (0)
#define PLAYTYPE_BOOK          (1)
#define PLAYTYPE_TEXT          (2)
#define PLAYTYPE_MEDIA		   (3)
#define PLAYTYPE_BLUETOOTH	   (4)
#define PLAYTYPE_RESTAURANT	   (5)
#define PLAYTYPE_NEWS	       (6)
#define PLAYTYPE_JOKE	       (7)
#define PLAYTYPE_POETRY	       (8)


/*EVT */
#define EVT_KEY   		(1<<0)   
#define EVT_TCP_SEND    (1<<2)  
#define EVT_PLAYER_SWITCH  (1<<3)   
#define EVT_IAT_SEMANTIC  (1<<4)   
#define EVT_MEDIAPLAYER_ACTION  (1<<5)
#define EVT_TIMER_ACTION  (1<<6)
#define EVT_INTERRUPT  (1<<7)
#define EVT_PHONE_CALLING  (1<<9)
#define EVT_AGENT_TIMER  (1<<10)
#define EVT_BOOKPLAYER_ACTION  (1<<11)
#define EVT_MUSICPLAYER_ACTION  (1<<12)
#define EVT_NETWORK_CHANGE  (1<<13) /*couldn't modify!!!must be 13 */
#define EVT_PLAY_TEXT_END  (1<<14)
#define EVT_USER_DIALOG  (1<<15)
#define EVT_PLAY_TEXTBOOK (1<<16)


#define WAKEUP_BUF_SIZE   256

#define TYPE_SEMANTIC   0
#define TYPE_SEMANTIC_FAILED   1
#define TYPE_NOTIFY   2 /*interrupt notify */
#define TYPE_RESULT   3 /*interrupt result */

#define WIFICONFIG_GOT_PASSWORD   1
#define WIFICONFIG_CONNECT_SUCCESS   2
#define WIFICONFIG_CONNECT_FAIL   3

#define SEMANTIC_BUF_SIZE   1024

enum source_type
{
	SOURCE_TYPE_BT_PLAY = 1,
	SOURCE_TYPE_GST_PLAY,
	SOURCE_TYPE_INSTRUCTION_PAUSE,   
	SOURCE_TYPE_INSTRUCTION_REPLAY, 
	SOURCE_TYPE_INSTRUCTION_NEXT,
	SOURCE_TYPE_INSTRUCTION_PRE, 

	SOURCE_TYPE_PUSHCONTROLE_START=7,
	SOURCE_TYPE_PUSHCONTROLE_PAUSE,
	SOURCE_TYPE_PUSHCONTROLE_PRE, 
	SOURCE_TYPE_PUSHCONTROLE_NEXT,
	SOURCE_TYPE_STOP_BLUETOOTHMUSIC,
	SOURCE_TYPE_BLUETOOTH_PHONERINGING,
	SOURCE_TYPE_BLUETOOTH_PHONECALLING, 
	SOURCE_TYPE_BLUETOOTH_PHONETALKING, 
	SOURCE_TYPE_BLUETOOTH_PHONEHUNGUP, 
	SOURCE_TYPE_BLUETOOTH_PHONESTOP, 
	SOURCE_TYPE_BLUETOOTH_PHONE_CONNECT, 
	SOURCE_TYPE_BLUETOOTH_PHONE_DISCON, 
	SOURCE_TYPE_OTHERS,  
};

typedef struct _devStatus_st
{
    lock_t devLock; 
   	uint32 curStatus; 
   	//uint32 preStatus;/*reserved for used in the future */ 
   	uint16 action;
}devStatus_st;

typedef struct
{
    int32 type; /*0: short, 1:long */
    char s_keyval[50];
	lock_t keyLock;
}keyNode_t;

/*通讯录信息 */
typedef struct
{
   char user_name[50];
   char user_phone_num[50];
   char contactId[50];
   char version[50];
}phoneNode_t;

typedef struct
{
   char song_name[50];
   char singerName[50];
   uint16 status;
   //uint16 action;
   int current;
   int playing; // 1=playing ,0=stop
   int total; // total length of this music
   int sourceType;
   int volume;
   int playMode;
   int play_right_now;
   char* song_url;
   char * playid;
   char * msgTime;
   char * picUrl;
   char * isCpAuth;
   	lock_t songLock;
}songNode_t;

typedef struct
{
   char media_name[50];
   char* play_url;
   int sourceType;
   int playing; // 1=playing ,0=stop
   int play_right_now;
}mediaNode_t;

enum mediaType
{
	MEDIA_TYPE_NULL = 0,
	MEDIA_TYPE_MUSIC,
	MEDIA_TYPE_STORY,
	MEDIA_TYPE_NEWS,
	MEDIA_TYPE_JOKE,
	MEDIA_TYPE_HEALTH,
	MEDIA_TYPE_RADIO,
	MEDIA_TYPE_CROSSTALK,
	MEDIA_TYPE_DRAMA,
	MEDIA_TYPE_STORYTELLING,
	MEDIA_TYPE_HISTORY,
	MEDIA_TYPE_ANIMALCRIES,
};

typedef struct
{
	char serviceCode[32];
	char inputAsrText[256];
	char answerText[256];
	struct json_object *result;
}aiMessage_t;

typedef struct
{
	uint16 status;
    char* chapterUrl;
    char contentName[128];
    char chapterName[128];
	char nextId[20];
	char prevId[20];
	char chapterSize[10];
	char *payUrl;
	int  isFree;
	lock_t bookLock;
	//for play text book
	char announce[32];
	URL_FILE *handle;
	char curLine[512];
}bookNode_t;


typedef struct
{
    char* timerSetupTime;
    char* timer_name;
    char intent[50];
	lock_t timerLock;
}timerNode_t;

typedef struct
{
    int32 type; 
    int8* data;
    int32 len;
	lock_t recLock;
    //int32 no; /*record node number,reserved for used in the future */
}recNode_t;
	
typedef struct
{
    int32 type; 
    //int8 data[WAKEUP_BUF_SIZE];
    //int32 len;
	lock_t wakLock;
}wakeupNode_t;

typedef struct
{
    void** param;
}gatTimerSigVal_t;

typedef struct _evtInfo_st evtInfo_st;
typedef void (*statEvent_Cb)(evtInfo_st evtInfo);


typedef struct _evtInfo_st
{
    int32 event;
    union
    {
    	keyNode_t keySigVal;
		songNode_t mediaplayerSigVal;
		timerNode_t timerSigVal;
		recNode_t recSigVal;
		wakeupNode_t wakeSigVal;
		devStatus_st devStasSigVal;
		phoneNode_t phoneSigVal;
		bookNode_t bookplayerSigVal;
		//gatTimerSigVal_t gatTimerSigVal;
    }param;
}evtInfo_st;


int32 wakeupNodeInit(void);
uint32 getDevStatus(void);
int32  statusEvtHandle(iofSig_t *sig);
int32 isNetConnected(void);
int32 isCloudPhoneBusy(void);
int32 isBtPhoneBusy(void);
int32 getPlayType(void);
void evtDevStatusInit(void);
void setDevStatus(uint32 status, int8 trueFlag);
void sendInterruptEvt(int32 type);

/************************************************************
** FunctionName : evt_recNode_init
** Description  : recNode节点初始化
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
int evt_recNode_init(void);

/************************************************************
** FunctionName : evt_get_recNode
** Description  : 获取recNode全局节点
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
recNode_t* evt_get_recNode(void);

/************************************************************
** FunctionName : semanticEventHandle
** Description  : semantic语义事件处理
** Input Param  : pRecNode
** Output Param : 
** Return Value : 
**************************************************************/
void semanticEventHandle(recNode_t* pRecNode);

/************************************************************
** FunctionName : interruptEventHandle
** Description  : 唤醒中断事件处理
** Input Param  : pWakeNode
** Output Param : 
** Return Value : 
**************************************************************/
void interruptEventHandle(wakeupNode_t* pWakeNode);

#ifdef __cplusplus
}
#endif
#endif
