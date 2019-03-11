#ifndef _HOMESERVER_H_
#define _HOMESERVER_H_

#ifdef __cplusplus
extern "C"{ 
#endif

#include "common.h"
#include "ledring.h"
#include "tcp.h"
//#include "gatplatform.h"
#include "json_engine.h"


#define FACTORY 1

#if DEV
//dev
#define MIGU_SERVER_DOMAIN	"data.walking.komect.com"
#define MIGU_SERVER_PORT	28888
#define ENV		"dev"
#endif

#if DEMO
//demo
#define MIGU_SERVER_DOMAIN	"data.walking.komect.com"
#define MIGU_SERVER_PORT	18888
#define ENV		"demo"
#endif

#if FACTORY
#define MIGU_SERVER_DOMAIN	"music.komect.com"
#define MIGU_SERVER_PORT	8085
#define ENV		"factory"
#endif

#if TEST 
#define MIGU_SERVER_DOMAIN	"music.komect.com"
#define MIGU_SERVER_PORT	48888
#define USING_TEST_IP 1
#define ENV		"test"
#endif

#define VERSION "2.3.0"

#define MIGU_LOGSYSTEM_DOMAIN	"http://data.walking.komect.com:28080"
#define MIGU_LOGSYSTEM_UPLOADPATH      "/smartapp/logaccess/uploadLogByDev"
#define VERSION_FILE	"/etc/SW_VERSION"
#define RECBUF_LEN      20*1024 

#define MIGU_HEADER_LEN     4

#define MIGU_COMMUNICATION      0 /*login... */
#define MIGU_HEARTBEAT          1 /*heartbeat */
#define MIGU_SEMANTIC           2 /* semantic */
#define MIGU_PUSHDEVINFO        3 /* pushdevinfo */
#define MIGU_BOOKINFO           4 /* request book info */

#define MIGU_HEARTBEAT_FREQ    30
#define MIGU_GETPHONELIST_FREQ    5
#define MIGU_UPGRADEINFO_FREQ    (60*60*1)

// 4 play modes 
#define SINGLE 21
#define RANDOM 22
#define SEQUENCE 23
#define LIST 24

enum _msgType {
	LOGIN = 0,
	HEART_BEAT,
	PUSH_DEV_INFO,
	SEMANTIC,
	GET_IMS_INFO,
	GET_IMS_PHONE_LIST,
	PUSH_AI_MESSAGE,
	GET_MDEV_USER,
	PUSH_MUSICS,
	PUSH_CONTROL,
	SYNC_IMS_INFO,
	SYNC_IMS_BIND_STATUS,
	SYNC_IMS_PHONE_LIST,
	PUSH_MDEV_USER,
	GET_MUSICINFO_BYID,
	GET_DEV_INFO,
	GET_UPGRADE_INFO,
	PUSH_ALARM,
	MSG_TYPE_COUNT,
};

typedef struct _miguClientNode
{
    uint8 szDomain[DOMAIN_LEN_MAX];
    uint8 szIp[IP_LEN_MAX];
    uint32 port;
    int32 fd;
    uint8 *sendBuf;
    uint32 sendLen;
    int32 reconnTimes; 
	int8 step;/*0:tcp connected, 1:init finished, 2:disconnected from wlan,3:hasn't got mac */
	lock_t cliLock;
    void *arg;          
}miguClientNode_t;

struct PlayListInfo{
	json_object *json_play_list;
	int playIndex;
	int list_len;
	int playMode;
	int mediaType;
};

struct HomeDemoListInfo{
	json_object *demo_list;
	int demoIndex;
	int list_len;
};

typedef struct _cloudphone{
	pthread_mutex_t mutex;  // mutex for all member of this struct except cmd
	pthread_cond_t  condition;  
	pthread_t ThreadId;

	int status;
	int cmd;
}CloudPhone;


struct PlayListInfo g_play_list_info;
struct PlayListInfo g_yuyi_play_list;
struct HomeDemoListInfo g_homeDemo_list_info;

void *homeserverThread(void *arg);
void *mediaplayerThread(void *arg);
void mediaplayerEventHandle(mediaNode_t* pMediaPlayerNode);
void musicplayerEventHandle(songNode_t *pMediaPlayerNode);
void bookplayerEventHandle(bookNode_t *pBookPlayerNode);
void timerEventHandle(timerNode_t* ptimerNode, int is_from_file); 
void tcpEventHandle(tcpSendListNode_t* clientSndNode);
void *cloudphoneThread(void *arg);
int miguSemantic(int8* data, int32 len);
int miguTbook(int8* data, int32 len);
gatTimer_st* getHeartbeatTimer(void);
gatTimer_st* getGetPhoneListTimer(void);
gatTimer_st* getSemanticTimer(void);
miguClientNode_t* getMiguClientNode(void);
int miguHeartbeat(int32 fd);
int32 heartBeatClear(void);
int8 getBootFlag(void);
void setBootFlag(int8 flag);
int32 bootFlagTimerCb( gatTimer_st *pTimer );
int send_server_data(char * data, int32 type);

int package_json_pushdevinfo_action(void);
void setClientStep(int8 step);
void setClientFd(int32 fd);
int32 getClientFd(void);

gatTimer_st* getLEDTimer(void);
gatTimer_st* getBootFlagTimer(void);
gatTimer_st* getNetconfigTimer(void);
gatTimer_st* getRequestHandleTimer(void);

int play_textbook_handle(bookNode_t *pBookPlayerNode);
int ap_config_handle();


#ifdef __cplusplus
}
#endif

#endif /* _HOMESERVER_ */

