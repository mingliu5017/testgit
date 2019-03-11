#ifndef __JSON_PARA_H
#define __JSON_PARA_H

#include <json-c/json.h>//"json-c/json.h"
#include <dbus/dbus.h>
#include "base64.h"
#include "phonebook.h"
#include "gattypes.h"
#include "statusevent_manage.h"
#ifdef   __cplusplus
extern "C" {
#endif

struct RequestJsonStruct {
	const char* msgType;
//	const char* wifi_mac;
	struct json_object* data;
	struct json_object* raw;
//	const char* dev_version;
};

typedef struct _RequestMiguDevInfo {
	char* mac;// 蓝牙MAC
	char* smartDeviceId;//"11+蓝牙MAC"
	char* msisdn;//用户手机号
	char* uid;
	char* sim;
}RequestMiguDevInfo;

struct package_json {
	const char* msg_type;
	const char* wifi_mac;
	struct json_object* data;
	struct json_object* raw;
	const char* dev_version;
};

struct BookRequestInfo
{
	char *chapterId;
	char *chapterQuality;
	char *contentId;
	char *contentName;
	char *dev_version;
};

#define BTMAC_LEN 20
#define SMARTDEVICEID_LEN 22

typedef struct
{
	char btmac[BTMAC_LEN];
	char smartDeviceId[SMARTDEVICEID_LEN];
}btMacInfo_t;


struct json_object* package_json_msg_login(struct package_json* _package);
struct json_object* package_json_msg_heartbeat(struct package_json* _package);
struct json_object* package_json_msg_music(struct package_json* _package);
//extern int json_parser(struct g_data* p, const char * json_string);
struct json_object* package_json_msg_tbook(struct package_json* _package) ;
int json_make_asrText(struct RequestJsonStruct* request_data, char* data);
int json_parser(const char * json_string, int dataLen);
struct json_object* json_make_request_data(struct RequestJsonStruct* request_data);
struct json_object* json_make_upgrade_data(struct RequestJsonStruct* request_data);
char * get_elements_byID_from_list(json_object * musicInfos,int index,const char *key);
char * get_elements_from_list(json_object * musicInfos,int index,const char * key);
int get_index_byMode_from_list(int playMode,int list_size,int is_pre);
char * json_get_string_by_key(json_object * obj, const char * key );
songNode_t* update_songNode_by_index_from_list(json_object * musicInfos,int index);

int32 aiMessageInit(void);
int32 bookNodeInit(void);

void mediaNext(int is_prev);
int32 timerDataInit(void);
int32 songNodeInit(void);
bookNode_t* getbookDataNode(void);
mediaNode_t* getmediaNode(void);
void restaurantNext(void);
void storyTellingNext(void);
void restaurantPrev(void);
void storyTellingPrev(void);
int play_book_next(int is_prev);
//void setPoetryPlayingFlag(int flag);
void setDialogFlag(int8 flag);
int8 getDialogFlag(void);
int update_recallCallbackNo_to_file(char * recallNo, char* callbackNo, int type);
int  homeDemoHander(void);
int cloudphone_cmd_triger(int cmd);

#ifdef   __cplusplus
}
#endif
#endif
