#include <assert.h>
#include <string.h>
#include "json_engine.h"
#include "homeserver.h"
#include "aml_cloudphone_main.h"
#include "aml_playermanager.h"

//#include "simpleplayer.h"
//#include "cae.h"
#include "network_cfg.h"

#define TAG_NAME_FILE "/data/tag_name.json"
extern char* msgType[];
int g_play_net_ok_flag = 0;
int alarm_OK = 1;

char * get_string_by_path(json_object * jo1,const char * path);

songNode_t songData;
static mediaNode_t mediaNode;
static timerNode_t timerData;
static bookNode_t bookData;
static btMacInfo_t btMacInfo;
static RequestMiguDevInfo g_migu_devinfo;

struct json_object *restaurantInfoObject = NULL;
struct json_object *newsInfoObject = NULL;
struct json_object *storyTellingInfoObject = NULL;
struct json_object *jokeInfoObject = NULL;

struct json_object *poetryInfoObject = NULL;


static phoneNode_t phoneData;
static char *migu_databuf=NULL;
const int migu_buf_size = 52*1024;
aiMessage_t aiMessage;
extern gatTimer_st ledTimer;

extern CloudPhone * p_cloudphone;
extern pthread_mutex_t cloudphone_mutex;
extern pthread_cond_t  cloudphone_condition;  
extern pthread_t cloudphone_ThreadId;
extern int cloudphone_cmd;
extern int g_cloudphone_ok;

static int updateImsPhoneList(json_object * json_root);
static int get_recallCallback_number(char* action, char* getNumberName);

bookNode_t* getbookDataNode(void)
{
    return &bookData;
}

mediaNode_t* getmediaNode(void)
{
    return &mediaNode;
}

/**判断str1是否以str2结尾
 * 如果是返回1
 * 不是返回0
 * 出错返回-1
 * */
int is_end_with(const char *str1, char *str2)
{
    if(str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if((len1 < len2) ||  (len1 == 0 || len2 == 0))
        return -1;
    while(len2 >= 1)
    {
        if(str2[len2 - 1] != str1[len1 - 1])
            return 0;
        len2--;
        len1--;
    }
    return 1;
}

int restaurantIndex = 0;
void restaurantNext(void)
{
	int restauranArraySize = 0;
	char answerText[512] = {0};
	struct json_object *obj;
	char* m_city = NULL;
	char* m_area = NULL;
	char* m_address = NULL;
	char* m_name = NULL;
	json_object * restaurantInfo;
	json_pointer_get(restaurantInfoObject,"/restaurantInfo", &restaurantInfo);
	restauranArraySize = json_object_array_length(restaurantInfo);	
	printf("%s:size=%d,restaurantIndex=%d\n", __func__, restauranArraySize, restaurantIndex);
	if (restaurantIndex > restauranArraySize-1)
	{
		//列表播放完毕
		restaurantIndex = restauranArraySize-1;
		sprintf(answerText,"没有下一家餐馆了,请重新搜索");
		playermanager_play_text(answerText);
		return;
	}
	obj = json_object_array_get_idx(restaurantInfo, restaurantIndex);
	m_city = get_string_by_path(obj,"/city");
	m_area = get_string_by_path(obj,"/area");
	m_address = get_string_by_path(obj,"/address");
	m_name = get_string_by_path(obj,"/name");
	if ((m_city == NULL)||(m_area == NULL)||(m_address == NULL)||(m_name == NULL)) {
		printf("[err] get restaurant node fail!\n ");
		return;
	}
	//obj = json_object_array_get_idx(restaurantInfo, restaurantIndex);
	restaurantIndex += 1;
	sprintf(answerText,"为你找到,%s%s%s,%s", 
		m_city, 
		m_area, 
		m_address,
		m_name 
	);	
	setDevStatus(DEV_STATUS_PLAYRESTAURANT, 1);
	playermanager_play_text(answerText);
}

void restaurantPrev(void)
{
	int restauranArraySize = 0;
	char answerText[512] = {0};
	struct json_object *obj;
	char* m_city = NULL;
	char* m_area = NULL;
	char* m_address = NULL;
	char* m_name = NULL;
	json_object * restaurantInfo;
	json_pointer_get(restaurantInfoObject,"/restaurantInfo", &restaurantInfo);
	restauranArraySize = json_object_array_length(restaurantInfo);	
	printf("%s:size=%d,restaurantIndex=%d\n", __func__, restauranArraySize, restaurantIndex);
	if (restaurantIndex < 0)
	{
		//列表播放完毕
		restaurantIndex = 0;
		sprintf(answerText,"没有上一家餐馆了,请重新搜索");
		playermanager_play_text(answerText);
		return;
	}
	obj = json_object_array_get_idx(restaurantInfo, restaurantIndex);
	m_city = get_string_by_path(obj,"/city");
	m_area = get_string_by_path(obj,"/area");
	m_address = get_string_by_path(obj,"/address");
	m_name = get_string_by_path(obj,"/name");
	if ((m_city == NULL)||(m_area == NULL)||(m_address == NULL)||(m_name == NULL)) {
		printf("[err] get restaurant node fail!\n ");
		return;
	}
	//obj = json_object_array_get_idx(restaurantInfo, restaurantIndex);
	restaurantIndex -= 1;
	if(restaurantIndex <= 0)
	{
		restaurantIndex = 1;
	}
	sprintf(answerText,"为你找到,%s%s%s,%s", 
		m_city, 
		m_area, 
		m_address,
		m_name 
	);	
	setDevStatus(DEV_STATUS_PLAYRESTAURANT, 1);
	playermanager_play_text(answerText);
}

void mediaNext(int is_prev)
{
	char answerText[512] = {0};
	mediaNode_t * p_mediaNode = &mediaNode;
	int32 type;
	int index;
	int ret;
	char* media_URL;
	if(g_play_list_info.json_play_list == NULL)
	{
		if (p_mediaNode->sourceType == MEDIA_TYPE_ANIMALCRIES)
		{
			gatPrintf( GAT_DEBUG,"play animalcries end.\n");	
		}
		else
		{
			playermanager_play_text("节目播放结束");
		}

		//play_release();
		setDevStatus(DEV_STATUS_PLAYING, 0);
		setDevStatus(DEV_STATUS_IDLE, 1);
		return ;
	}
	if (is_prev==1)
	{
		index = g_play_list_info.playIndex -1;
	}
	else
	{
		index = g_play_list_info.playIndex +1;
	}
	printf("mediaNext index = %d\n", index);
	// reach first
	if(index < 0)
	{

		playermanager_play_text("没有上一个了，您可以对我说：下一个");

		//play_release();
		//setDevStatus(DEV_STATUS_PLAYING, 0);
		//setDevStatus(DEV_STATUS_IDLE, 1);
		return ;
	}
	//end of list
	else if (index >= g_play_list_info.list_len)
	{
		playermanager_play_text("节目播放结束，换个其它的吧");
		//play_release();
		setDevStatus(DEV_STATUS_PLAYING, 0);
		setDevStatus(DEV_STATUS_IDLE, 1);
		return ;
	}
	//play list have next.
	g_play_list_info.playIndex = index;

	struct json_object *obj = json_object_array_get_idx(
								g_play_list_info.json_play_list, 
								g_play_list_info.playIndex );
	switch (g_play_list_info.mediaType)
	{

		case MEDIA_TYPE_NEWS:
		case MEDIA_TYPE_STORYTELLING:
		case MEDIA_TYPE_HEALTH:
		case MEDIA_TYPE_RADIO:
			media_URL = get_string_by_path(obj, "/url");
			if (media_URL == NULL) {
				printf("[err] get media URL fail!\n");
				return;
			}
			break;
		case MEDIA_TYPE_JOKE:
			media_URL = get_string_by_path(obj, "/mp3Url");
			break;
		default:
			media_URL = get_string_by_path(obj, "/url");
			if (media_URL == NULL) {
				printf("[err] get default media URL fail!\n");
				return;
			}
		break;

	}
	//char* new_title = get_string_by_path(obj, "/title");
	if (media_URL != NULL) {
		p_mediaNode->sourceType = g_play_list_info.mediaType;
		setDevStatus(DEV_STATUS_PLAYING, 1);
		//update_gst_url(media_URL); //mark
	}
	else {
		 type = TYPE_RESULT;
		 sendInterruptEvt(type);
		 return;
	}
}


static char* getDeviceId(char* btMac)
{
	if(strlen(btMac) > sizeof(btMacInfo.smartDeviceId))
	{
		return NULL;
	}
	memset(&btMacInfo.smartDeviceId[0], 0, sizeof(btMacInfo.smartDeviceId));
	sprintf(btMacInfo.smartDeviceId,"11%s",btMac);
	
	//memcpy(&btMacInfo.smartDeviceId[0], (char*)deviceId, len);
	return btMacInfo.smartDeviceId;
}

//char * get_string_by_path(json_object * jo1,const char * path);

int migu_play_list_init()
{
	memset(&g_play_list_info,0,sizeof(struct PlayListInfo));
	memset(&g_play_list_info,0,sizeof(struct PlayListInfo));

	g_play_list_info.json_play_list= NULL;
	g_play_list_info.playIndex = 0;
	g_play_list_info.playMode = LIST;
	g_play_list_info.list_len= 0;
	g_play_list_info.mediaType= MEDIA_TYPE_NULL;
}
char * json_make_migu_devinfo( void )
{
	int ret = 0;
	char * json_string;
	json_object *json_root;

	ret = migumusicInit();
	if(ret < 0)
	{
		return NULL;
	}
	json_root= json_object_new_object();
	json_object_object_add(json_root, "mac", json_object_new_string(g_migu_devinfo.mac));
	json_object_object_add(json_root, "smartDeviceId", json_object_new_string(g_migu_devinfo.smartDeviceId));
#if 0
	if (g_migu_devinfo.smartDeviceId == NULL)
	{
		printf("%s:%d.\n",__func__,__LINE__);
		g_migu_devinfo.smartDeviceId = iofMalloc(22); 
		if(g_migu_devinfo.smartDeviceId == NULL)
		{
			jsonPrintf( GAT_ERROR,"%s:%d malloc error.\n",__func__,__LINE__);
			return NULL;
		}
		memset(g_migu_devinfo.smartDeviceId,0,22);
		sprintf(g_migu_devinfo.smartDeviceId,"11%s",g_migu_devinfo.mac);
		json_object_object_add(json_root, "smartDeviceId", json_object_new_string(g_migu_devinfo.mac));
		printf("%s:%d. BT mac:%s\n",__func__,__LINE__,g_migu_devinfo.smartDeviceId);
	}
	else
	{
		printf("%s:%d.\n",__func__,__LINE__);
		json_object_object_add(json_root, "smartDeviceId", json_object_new_string(g_migu_devinfo.smartDeviceId));
	}
#endif
	if (g_migu_devinfo.msisdn != NULL)
	{
		json_object_object_add(json_root, "msisdn", json_object_new_string(g_migu_devinfo.msisdn));
	}

	if (g_migu_devinfo.uid!= NULL)
	{
		json_object_object_add(json_root, "uid", json_object_new_string(g_migu_devinfo.uid));
	}
	if (g_migu_devinfo.sim!= NULL)
	{
		json_object_object_add(json_root, "sim", json_object_new_string(g_migu_devinfo.sim));
	}
	json_string = (char*)json_object_to_json_string(json_root);
	printf("%s:%s\n",json_string);
	printf("memberphone = %s\n",g_migu_devinfo.msisdn );

	return json_string;
}

 static int32 upgradeInfoTimerCb( gatTimer_st *pTimer )
 {	 
	 int32 ret=-1;
	 int32 fd = (int32)pTimer->param;
	 miguPrintf( GAT_DEBUG,"fd:%d.\n", pTimer->param);
 
	 if(fd < 0)
	 {
		 miguPrintf( GAT_ERROR,"%s fd invalid!\r\n",__FUNCTION__ );
		 return 0;
	 }
	 ret = miguUpgradeInfo((int32)pTimer->param);
	 if(ret < 0)
	 {
		 return 0;
	 }
	 return MIGU_UPGRADEINFO_FREQ;
 }
 
 static int32 getPhoneListTimerCb( gatTimer_st *pTimer )
 {	 
	 int32 ret=-1;
	 static int32 tryNumber=0;
	 
	 int32 fd = (int32)pTimer->param;
	 miguPrintf( GAT_DEBUG,"%s, fd:%d.\n", __FUNCTION__,pTimer->param);
	 if(fd < 0)
	 {
		 miguPrintf( GAT_ERROR,"%s fd invalid!\r\n",__FUNCTION__ );
		 return 0;
	 }
	 ret = getImsPhoneListAction((int32)pTimer->param);
	 if (ret < 0)
	 {
		 if(tryNumber++>=3)
		 {
			 miguPrintf( GAT_ERROR,"%s getImsPhoneListAction fail, try out!\r\n",__FUNCTION__ );
			 return 0;
		 }
		 miguPrintf( GAT_ERROR,"%s getImsPhoneListAction fail\r\n",__FUNCTION__ );
		 return MIGU_GETPHONELIST_FREQ;
	 }
	 return 0;
 }

struct json_object* json_make_request_data(struct RequestJsonStruct* request_data)
{
	char * devKey ;
	char btmac_format[20];
	char _devkey[40];
	char *res;
	char *json_string;
	char *pBTmac=NULL;

	json_object *json_root;
	json_root= json_object_new_object();
	json_object_object_add(json_root, "msgType", json_object_new_string(request_data->msgType));

	//BTMac
	memset(btmac_format,0,20);
	pBTmac = get_BT_Mac_addr();
	if(pBTmac != NULL)
	{
		memcpy(btmac_format, (char*)pBTmac, strlen(pBTmac));
	}
	//json_object_object_add(json_root, "BTMac", json_object_new_string(request_data->BTMac);
	json_object_object_add(json_root, "did", json_object_new_string(btmac_format));
	//

	//devkey
	memset(_devkey,0,40);
	sprintf(_devkey,"MUSES_DEVICE%s",btmac_format);
	devKey=base64_encode(_devkey);
	json_object_object_add(json_root, "devKey", json_object_new_string(devKey));

	//data
	//json_object_object_add(request_data->data, "isSemantic", json_object_new_int(1));
	json_object_object_add(json_root, "data", request_data->data);
	json_object_object_add(json_root, "devVersion", json_object_new_string(VERSION));
	json_object_object_add(json_root, "raw", request_data->raw);
	
	return json_root;

}


struct json_object* json_make_upgrade_data(struct RequestJsonStruct* request_data)
{
	char * devKey ;
	char btmac_format[20];
	char _devkey[40];
	char *res;
	char *json_string;
	char *pBTmac=NULL;

	json_object *json_root;
	json_root= json_object_new_object();
	json_object_object_add(json_root, "msgType", json_object_new_string(request_data->msgType));

	//BTMac
	memset(btmac_format,0,20);
	pBTmac = get_BT_Mac_addr();
	if(pBTmac != NULL)
	{
		memcpy(btmac_format, (char*)pBTmac, strlen(pBTmac));
	}
	//json_object_object_add(json_root, "BTMac", json_object_new_string(request_data->BTMac);
	json_object_object_add(json_root, "did", json_object_new_string(btmac_format));

	//devkey
	memset(_devkey,0,40);
	sprintf(_devkey,"MUSES_DEVICE%s",btmac_format);
	devKey=base64_encode(_devkey);
	json_object_object_add(json_root, "devKey", json_object_new_string(devKey));
	//data

	json_object_object_add(json_root, "devVersion", json_object_new_string(VERSION));
	
	return json_root;

}


int json_make_asrText(struct RequestJsonStruct* request_data, char* data)
{
	struct json_object *p_data=NULL, *p_raw=NULL;
	//put data
	p_data = json_object_new_object();
	json_object_object_add(p_data, "inputAsrText", json_object_new_string((char*)data));
	json_object_object_add(p_data, "isSemantic", json_object_new_int(1));
	request_data->data =p_data;
	//put raw
	p_raw= json_object_new_object();
	json_object_object_add(p_raw, "msgCode", json_object_new_string("inputAsrText"));
	request_data->raw = p_raw;
	if(request_data->data==NULL || request_data->raw ==NULL)
	{
		return -1;
	}
	return 0;
}

char * json_get_string_by_key(json_object * obj, const char * key )
{
	char * key_str;

	json_object * parse_key_obj = json_object_object_get(obj, key);
	key_str =(char *)json_object_get_string(parse_key_obj);

	return key_str;
}

int json_get_int_by_key(json_object * obj, const char * key )
{
	int key_int;

	json_object * parse_key_obj = json_object_object_get(obj, key);
	key_int = json_object_get_int(parse_key_obj);

	return key_int;
}
int msgType_to_int(const char * in_msgType)
{
	unsigned int i, msgTypeCount;
	msgTypeCount = MSG_TYPE_COUNT;
	
	for (i=0; i < msgTypeCount; i++){
		if (strcmp(msgType[i], in_msgType) == 0)
			return i;
	}
	return -1;
}

int update_memberPhone_to_file(char * memberPhone)
{
	json_object  * data_root;
	char * old_memberPhone;
	int ret;

	gatPrintf( GAT_DEBUG,"get memberPhone: %s\n",memberPhone);
	//have old data.	
	if (access(DATA_FILE, F_OK) == 0)
	{
		data_root = json_object_from_file(DATA_FILE);
		if(data_root != NULL)
		{
			json_object_object_del(data_root, "memberPhone");

			if(memberPhone != NULL && strlen(memberPhone) > 5)
			{
				json_object_object_add(data_root,"memberPhone",json_object_new_string(memberPhone));
			}
		}
		else{
		remove(DATA_FILE);
		data_root = json_object_new_object(); 
		json_object_object_add(data_root,"memberPhone",json_object_new_string(memberPhone));
		}
	}
	else
	{
		data_root = json_object_new_object(); 
		json_object_object_add(data_root,"memberPhone",json_object_new_string(memberPhone));
	}
	ret = json_object_to_file_ext(DATA_FILE,data_root,2);
	if(ret < 0)
	{
		gatPrintf( GAT_DEBUG,"json_data write to file error.\n");
		printf("%s\n", json_util_get_last_err());
	}
	return ret;

}
int login_hander(json_object * json_root)
{
	char * memberPhone =NULL; 
	int ret;
	int ssid;
	int update_flag;
	ssid = is_ssid_QLINK();

	gatPrintf( GAT_DEBUG,"ssid is QLINK ?: %d\n",ssid);
	if (ssid==1)
	{
		return -1;
	}
	else
	{
		iofTimerInit();
		playermanager_play_file("/media/sound/wificonfig_ok.pcm");
		//led_all_off_mute();
		setBootFlag(1);
		setDevStatus(DEV_STATUS_CONNECTED, 1);
		gatTimerAdd(getBootFlagTimer(), "bootFlagTimer",13,bootFlagTimerCb,NULL);
		//网络状态ok
		heartBeatClear();
		jsonPrintf( GAT_DEBUG,"%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));
		gatTimerAdd(getGetPhoneListTimer(), "getPhoneListTimer",3,getPhoneListTimerCb,(void*)(getMiguClientNode()->fd));
		gatTimerAdd(getUpgradeInfoTimer(), "upgradeInfoTimer",60,upgradeInfoTimerCb,(void*)(getMiguClientNode()->fd));

		memberPhone = get_string_by_path(json_root,"/data/memberPhone"); 
		ret = update_memberPhone_to_file(memberPhone);

		//aml_enterCloudphone();

		update_timer_from_file();

		return ret;
	}
}
int heartBeat_hander(json_object * json_root)
{
#if 0
	char* recode = NULL;
	char* msg = NULL;
	recode = get_string_by_path(json_root,"/recode");
	msg = get_string_by_path(json_root,"/msg");
	if ((recode == NULL)||(msg == NULL)) {
		printf("[err] heartBeat recode or msg is NULL!\n");
		return -1;
	}
	gatPrintf( GAT_DEBUG,"recode:%s, msg:%s\n", recode, msg);
	if (!strcmp(recode, "6")) {
		playermanager_play_text(msg);
		return -1;
	}
#endif
	heartBeatDecrease();
	return 0;
}


int pushDevInfo_hander(json_object * json_root)
{
	//printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));
	//jsonPrintf( GAT_DEBUG,"%s enter!\n",__func__);
	gatTimerDel(getRequestHandleTimer());
	return 0;
}

char * get_string_by_path(json_object * jo1,const char * path)
{
	int res = 0;
	json_object  *jo2 = NULL;

	res = json_pointer_get(jo1, path, &jo2);

	if ((res < 0)||(jo2 == NULL)) { 
		printf("[err], get string node fail!\n");
		return NULL;
	}
	
	if (json_object_is_type(jo2, json_type_string))
		return (char*)json_object_get_string(jo2);
	else {
		gatPrintf( GAT_DEBUG,"couldn't find %s path\n", path);
		return NULL;
	}
}

char * get_null_by_path(json_object * jo1,const char * path)
{
	json_object  *jo2 = NULL;
	int res = 0;

	res = json_pointer_get(jo1, path, &jo2);
	if ((res < 0)||(jo2 == NULL)) {
		printf("[err], get null node fail!\n");
		return NULL;
	}
	
	if (json_object_is_type(jo2, json_type_null))
		return (char*)json_object_get_string(jo2);
	else {
		gatPrintf( GAT_DEBUG,"couldn't find %s path\n", path);
		return NULL;
	}
}

int get_int_by_path(json_object * jo1,const char * path)
{
	json_object  *jo2 = NULL;
	int res = 0;

	res = json_pointer_get(jo1, path, &jo2);
	if ((res < 0)||(jo2 == NULL)) {
		printf("[err], get int node fail!\n");
		return -1;
	}
	
	if (json_object_is_type(jo2, json_type_int))
		return json_object_get_int(jo2);
	else {
		gatPrintf( GAT_DEBUG,"json content type error.\n");
		return -1;
	}
	return 0;
}

int request_book_info_by_Id(struct BookRequestInfo * book_request_info) 
{
	struct json_object* json_data, *p_data, *p_raw;
	int ret;
	char* json_str_tbook;
	struct RequestJsonStruct request_json_struct;
	int32 type=MIGU_BOOKINFO;
	printf("## enter %s.\n",__func__);

	if (book_request_info == NULL ) {
		gatPrintf( GAT_DEBUG,"Error: pointer was NULL.\n");
		return -1;
	}
		
#if 0
	if (NULL == book_request_info->chapterQuality) {
		book_request_info->chapterQuality = "1" ;
	}
#endif

	/*make semantic str*/
	//put msgType
	request_json_struct.msgType = msgType[GET_MUSICINFO_BYID];

	//put data
	p_data = json_object_new_object();
	if (NULL == p_data) {
		gatPrintf( GAT_DEBUG,"new json _data object failed.\n");
		return -1;
	}
	
	json_object_object_add(p_data, "chapterId", json_object_new_string(book_request_info->chapterId));
	json_object_object_add(p_data, "chapterQuality", json_object_new_string("1"));
	request_json_struct.data = p_data;

	//put raw
	p_raw= json_object_new_object();
	json_object_object_add(p_raw, "msgCode", json_object_new_string("get bookinfo by id"));
	request_json_struct.raw = p_raw;

	//make json_data
	json_data = json_make_request_data(&request_json_struct);

	//check json_data
	if (NULL == json_data) {
		gatPrintf( GAT_DEBUG,"make json_data object failed.\n");

		//g_release_threads(g_comm);
		return -1;
	}
	json_str_tbook= (char*)json_object_to_json_string(json_data);
	gatPrintf( GAT_DEBUG,"json_semantic_str:\n%s\n", json_str_tbook);
	send_server_data(json_str_tbook, type);
	
	return 0;		
}

int search_callback(void * data, int argc, char ** argv, char ** azColName) {
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

json_object * get_migu_buf_by_musicID(char * migu_musicId)
{
	json_object *json_migu_data;
	json_object *musicInfo;
	//get migu_databuf by musicId
	memset(migu_databuf,0,4*1024);	
	get_musicInfo(migu_musicId, migu_databuf);
	if (migu_musicId == NULL)
	{
		printf("%s:%d: get musicURL from json error.\n",__func__,__LINE__);
		return NULL;
	}
	//migu_databuf parse to json_object
	json_migu_data = json_tokener_parse(migu_databuf); 
	if (json_migu_data == NULL )
	{
		printf("%s:%d: get migu_databuf not json object.\n",__func__,__LINE__);
		return NULL;
	}
	//gatPrintf( GAT_DEBUG,"id:%s\nmigu_return: \n%s\n",migu_musicId,json_object_to_json_string_ext(json_migu_data ,2));

	json_pointer_get(json_migu_data , "/musicInfo", &musicInfo);

	return musicInfo; 
}

json_object * get_migu_buf_by_key(char * migu_key);
json_object * get_migu_buf_from_list (json_object * music_list,int index)
{
	char tmp_path[128];
	int list_len;
	int i;
	char * migu_musicId;

	char migu_databuf[4*1024];
	//printf("%s: in data: \n%s\n",__func__,json_object_to_json_string_ext(music_list,2));
	if (! json_object_is_type(music_list,json_type_array))
	{
		printf("%s:%d: get musicInfos from json error.\n",__func__,__LINE__);
		return NULL;
	}
	if (index >= g_play_list_info.list_len)
	{
		printf("%s:%d: index is bigger than playlist length.\n",__func__,__LINE__);
		return NULL;
	}
	if (index < 0 )
	{
		printf("%s:%d: index is -1, end of music list?\n",__func__,__LINE__);
		return NULL;
	}

	// get musicId
	memset(tmp_path,0,128);
	sprintf(tmp_path,"/%d/musicId",index);
	migu_musicId = get_string_by_path(music_list,tmp_path);
	if (migu_musicId != NULL)
	{
	//	printf("%s:%d: get musicURL from json error.\n",__func__,__LINE__);
		return get_migu_buf_by_musicID(migu_musicId);
	}
	else
	{
		char* artist_name=NULL;
		char * song_name=NULL;

		//play music by keys
		memset(tmp_path,0,128);
		sprintf(tmp_path,"/%d/song",index);
		song_name = get_string_by_path(g_play_list_info.json_play_list,tmp_path);

		memset(tmp_path,0,128);
		sprintf(tmp_path,"/%d/artist",index);
		artist_name = get_string_by_path(g_play_list_info.json_play_list,tmp_path);
		if (song_name == NULL ||artist_name == NULL)
		{
			printf("get song name or artist name is NULL.\n");
			return NULL;
		}
		memset(tmp_path,0,128);
		sprintf(tmp_path,"%s%s",artist_name,song_name );
		printf("%s:miguKey:%s\n",__func__,tmp_path);
		return get_migu_buf_by_key(tmp_path);
	}

}

songNode_t * update_songNode_by_migudata(json_object * json_migu_data)
{
	char * musicId;
	char * play_URL;
	char * tmp_url;
	char * picUrl;
	char * song_name;
	char * singerName;
	char * playid;
	char * isCpAuth;
	char  volume[5];

	songNode_t* p_songNode = &songData;

	//printf("%s: in data: \n%s\n",__func__,json_object_to_json_string_ext(json_migu_data,2));
	isCpAuth = get_string_by_path(json_migu_data, "/isCpAuth"); 
	if(strcmp(isCpAuth,"0")==0) 
	{
		printf("%s: get isCpAuth return NULL",__func__);
		p_songNode->isCpAuth=isCpAuth;
		return p_songNode;
	}

	musicId = get_string_by_path(json_migu_data, "/musicId"); 
	if (musicId == NULL)
	{
		printf("%s: get musicId return NULL",__func__);
		return NULL;
	}

	song_name = get_string_by_path(json_migu_data, "/musicName"); 
	if (song_name == NULL)
	{
		printf("%s: get song_name return NULL",__func__);
		return NULL;
	}
	if(strcmp(isCpAuth,"1")==0) 
	{
		if (NULL != (tmp_url= get_string_by_path(json_migu_data, "/hqListenUrl")))
		{
			play_URL = tmp_url;
			gatPrintf( GAT_DEBUG,"get hq_url:\n%s\n",play_URL);
		}
		else if (NULL != (tmp_url= get_string_by_path(json_migu_data, "/sqListenUrl")))
		{
			play_URL = tmp_url;
			gatPrintf( GAT_DEBUG,"get sq_url:\n%s\n",play_URL);
		}
		else if (NULL != (tmp_url= get_string_by_path(json_migu_data, "/listenUrl")))
		{
			play_URL = tmp_url;
			gatPrintf( GAT_DEBUG,"get pq_url:\n%s\n",play_URL);
		}
		else
		{
			printf("%s: get play_URL return NULL",__func__);
			return NULL;
		}
	}
	else{
		play_URL = ""; 
	}
	picUrl = get_string_by_path(json_migu_data, "/picUrl"); 
	if (picUrl == NULL)
	{
		printf("%s: get picUrl return NULL",__func__);
		return NULL;
	}

	//iofLock(&p_songNode->songLock);
	memset(p_songNode->song_name,0,50);
	sprintf(p_songNode->song_name, "%s", song_name);

	singerName = get_string_by_path(json_migu_data, "/singerName"); 
	memset(p_songNode->singerName,0,50);
	sprintf(p_songNode->singerName, "%s", singerName);

	p_songNode->playid = musicId;
	p_songNode->isCpAuth=isCpAuth;
	p_songNode->song_url = play_URL;
	p_songNode->picUrl = picUrl;
	p_songNode->status = DEV_STATUS_PLAYING;
	p_songNode->playing= 1;
	p_songNode->playMode = g_play_list_info.playMode;
	p_songNode->msgTime = get_time_string();
	p_songNode->sourceType = 1;

	memset(volume,0,5);
	read_file("/etc/system_volume",(char *)volume);
	if (volume != NULL)
	{
		p_songNode->volume = atoi(volume);
	}

	//iofUnlock(&p_songNode->songLock);
	return p_songNode;
}

char * get_elements_byID_from_list(json_object * music_list,int index,const char * key)
{
	int list_len;
	int i;
	json_object *json_migu_data;
	char * value;
	char tmp_path[128];

	json_migu_data = (json_object *)get_migu_buf_from_list (music_list,index);
	if (json_migu_data == NULL )
	{
		printf("%s:%d: get migu_databuf not json object.\n",__func__,__LINE__);
		return NULL;
	}

	memset(tmp_path,0,128);
	sprintf(tmp_path,"/%s",key);
	value = get_string_by_path(json_migu_data, tmp_path); 
	if (value != NULL) {
		gatPrintf( GAT_DEBUG," value :\n%s\n",value);
		return value ;
	}
	else{
		gatPrintf( GAT_DEBUG,"can't find this music.\n");
		return NULL;
	}

	return NULL;
}

songNode_t* update_songNode_by_index_from_list(json_object * musicInfos,int index)
{

	json_object *json_migu_data;
	 printf("%s: index = %d\n",__func__,index);
	json_migu_data = get_migu_buf_from_list(musicInfos,index);
	if (json_migu_data == NULL )
	{
		return NULL;
	}
	return update_songNode_by_migudata(json_migu_data );
}

/*
 * parms: 
 * playMode
 * list_size
 * is_pre: 
 * is_pre = 0, next; 
 * is_pre = 1, prev; 
 * is_pre = 2,next on single mode.
 * is_pre = 3,prev on single mode.
 * */
//return -1 if get end of list
int get_index_byMode_from_list(int playMode,int list_size,int is_pre)
{
	int index;
	switch(playMode)
	{
		case SINGLE: 
			if (is_pre == 2)
			{
				if (g_play_list_info.playIndex >= (list_size-1))
					index = 0;
				else
					index = g_play_list_info.playIndex+1;
			}
			else if (is_pre == 3)
			{
				if (g_play_list_info.playIndex ==0)
					index = list_size-1;
				else
					index = g_play_list_info.playIndex-1;
			}
			else
				index = g_play_list_info.playIndex;
			break;
	
		case RANDOM: 
			srand((int)time(0));
			index =((int)rand())%list_size;
			break;
		// 列表循环播放
		case SEQUENCE:
		case LIST: 
			// prev
			if (is_pre==1 || is_pre ==3)
			{
				if (g_play_list_info.playIndex ==0)
					index = list_size-1;
				else
					index = g_play_list_info.playIndex-1;
			}
			//next
			else
			{
				if (g_play_list_info.playIndex >= (list_size-1))
					index = 0;
				else
					index = g_play_list_info.playIndex+1;
			}
			break;
	}
	printf("%s: playMode=%d,index=%d\n",__func__,playMode,index);
	return index;
}

char * get_elements_from_list(json_object * musicInfos,int index,const char * key)
{
	char * str_value;
	char tmp_path[128];

	if (! json_object_is_type(musicInfos,json_type_array))
	{
		printf("%s:%d: get musicInfos from json error.\n",__func__,__LINE__);
		return NULL;
	}
	if (index >= g_play_list_info.list_len)
	{
		printf("%s:%d: index is bigger than playlist length.\n",__func__,__LINE__);
		return NULL;
	}
	memset(tmp_path,0,128);
	sprintf(tmp_path,"/%d/%s",index,key);
	str_value = get_string_by_path(musicInfos,tmp_path);
	if (str_value == NULL)
	{
		printf("%s:%d: get musicURL from json error.\n",__func__,__LINE__);
		return NULL;
	}
	gatPrintf( GAT_DEBUG,"str_value :\n%s\n",str_value );
	return str_value ;
}

int play_songNode(songNode_t* p_songNode,int need_answer)
{
	json_object *ai_result= NULL;
	json_object * json_root=NULL;
	aiMessage_t* p_aiMessage = &aiMessage;
	int32 type;
	int ret;
	char tmpBuf[1024];

	printf("%s: need_answer:%d\n",__func__,need_answer);

	if(strcmp(p_songNode->isCpAuth,"0")==0) 
	{
		memset(tmpBuf,0,1024);
		sprintf(tmpBuf, "%s", "咪咕音乐正在努力引入版权");
		playermanager_play_text(tmpBuf);
		return -1;
#if 0
		//update aiMessage
		sprintf(p_aiMessage->serviceCode,"%s","musicX");
		ai_result = json_object_new_object();
		if(ai_result ==NULL)
		{
			return -1;
		}
		sprintf(tmpBuf,"/musicInfos/%d",g_play_list_info.playIndex);
		json_pointer_get(json_root,tmpBuf, &m_musicInfos);
		if (m_musicInfos == NULL)
		{
			playermanager_play_text("查询歌曲信息失败");
			return -1;
		}

		playid = get_string_by_path(m_musicInfos,"/musicId");
		musicName= get_string_by_path(m_musicInfos,"/musicName");
		singerName= get_string_by_path(m_musicInfos,"/singerName");
		json_object_object_add(ai_result, "musicId", json_object_new_string(playid));
		json_object_object_add(ai_result, "musicSource", json_object_new_int(1));
		json_object_object_add(ai_result, "isCpAuth", json_object_new_int(0));
		json_object_object_add(ai_result, "musicName", json_object_new_string(musicName));
		json_object_object_add(ai_result, "singerName", json_object_new_string(singerName));

		gatPrintf( GAT_DEBUG,"ai_result:%s\n",json_object_to_json_string_ext(ai_result,2));
		p_aiMessage->result = ai_result;
		memset(p_aiMessage->answerText,0,256);
		sprintf(p_aiMessage->answerText,"%s",tmpBuf);
		ret = package_json_pushaiMessage_action();
		printf("%s:%d pushaiMessage return: %d\n", __func__,__LINE__,ret);

#endif
	}

	if(	p_songNode->song_name !=NULL &&  
		p_songNode->singerName !=NULL && 
		p_songNode->song_url !=NULL)
	{
		statusEventNotify(EVT_MUSICPLAYER_ACTION, (songNode_t*)&p_songNode);
		//need say feedback words.
		if (need_answer)
		{
			memset(tmpBuf,0,1024);
			sprintf(tmpBuf, "%s%s%s%s", "好的马上为您播放",
								p_songNode-> singerName , 
								"的", 
								p_songNode->song_name);	
			playermanager_play_text(tmpBuf);

			//update aiMessage
			sprintf(p_aiMessage->serviceCode,"%s","musicX");
			ai_result = json_object_new_object();
			if(ai_result ==NULL)
			{
				return -1;
			}
			json_object_object_add(ai_result, "musicId", json_object_new_string(p_songNode->playid));
			json_object_object_add(ai_result, "musicSource", json_object_new_int(1));
			json_object_object_add(ai_result, "isCpAuth", json_object_new_int(1));
			json_object_object_add(ai_result, "musicName", json_object_new_string(p_songNode->song_name));
			json_object_object_add(ai_result, "singerName", json_object_new_string(p_songNode->singerName ));

			p_aiMessage->result = ai_result;
			memset(p_aiMessage->answerText,0,256);
			sprintf(p_aiMessage->answerText,"%s",tmpBuf);
			ret = package_json_pushaiMessage_action();
			printf("%s:%d pushaiMessage return: %d\n", __func__,__LINE__,ret);
		}
	} else {
		gatPrintf( GAT_DEBUG,"play_URL/song_name/song_artist is NULL!!!!!\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);
		return -1;
	}
}

int parse_migu_buf(char * migu_databuf,int is_play_list)
{
	json_object * json_root=NULL;
	json_object * musicInfos= NULL;
	json_object * m_musicInfos = NULL;
	songNode_t* p_songNode = NULL;
	int32 type;
	int ret;
	char * play_URL;
	char * song_name;
	char * song_artist;
	char * playid;
	char * musicName;
	char * singerName;
	char  volume[5];

	//printf("%s:%d: is list?=%d in data: \n%s\n",__func__,__LINE__,
	//								is_play_list, migu_databuf);

	json_root = json_tokener_parse(migu_databuf); 
	if (json_root == NULL )
	{
		printf("%s:%d: convert  migu_databu to json_root error.\n",__func__,__LINE__);
		return -1;
	}

	//printf("%s:%d: is list?=%d in data: \n%s\n",__func__,__LINE__,
	//		is_play_list, json_object_to_json_string_ext(json_root,2));
	json_pointer_get(json_root, "/musicInfos", &musicInfos);
	if (! json_object_is_type(musicInfos,json_type_array))
	{
		printf("%s:%d: get musicInfos from json error.\n",__func__,__LINE__);
		printf("%s:%d: migu_data:\n%s\n",__func__,__LINE__,migu_databuf);
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}
	// 要播整个歌单。将歌单json信息添加到全局g_play_list_info
	if (is_play_list > 0)
	{
		if (g_play_list_info.json_play_list != NULL)
		{
			json_object_put(g_play_list_info.json_play_list);
		}
		g_play_list_info.json_play_list = musicInfos;
		g_play_list_info.list_len= json_object_array_length(musicInfos);
		g_play_list_info.playMode= RANDOM;
		g_play_list_info.playIndex= get_index_byMode_from_list(
									g_play_list_info.playMode,
									g_play_list_info.list_len,
									0);
		
	}
	//只播一首歌
	else
	{
		g_play_list_info.playIndex = 0;
		if( g_play_list_info.list_len <=0)
			g_play_list_info.list_len=1;
		printf("%s:%d: only play one URL.\n",__func__,__LINE__);
	}

	p_songNode = update_songNode_by_index_from_list(musicInfos,g_play_list_info.playIndex);
	if (p_songNode == NULL)
	{
		playermanager_play_text("查询歌曲信息失败");
		return -1;
	}
	play_songNode(p_songNode,1);
	return 0;
}

int32 timerDataInit(void)
{
    memset(&timerData,0,sizeof(timerNode_t));
    iofLockInit(&timerData.timerLock, KEYID_TIMER_SYNC);
	return 0;
}
timerNode_t * get_timerData()
{
	return &timerData;
}

int32 songNodeInit(void)
{
    memset(&songData,0,sizeof(songNode_t));
	songData.song_url=NULL;
	songData.playid=NULL;
	songData.msgTime=NULL;
	songData.picUrl=NULL;
    iofLockInit(&songData.songLock, KEYID_SONG_SYNC);
	return 0;
}

int32 aiMessageInit(void)
{
    memset(&aiMessage,0,sizeof(aiMessage_t));
	aiMessage.result= NULL;
  //  iofLockInit(&songData.songLock, KEYID_SONG_SYNC);
	return 0;
}

int32 bookNodeInit(void)
{
    memset(&bookData,0,sizeof(bookNode_t));
    iofLockInit(&bookData.bookLock, KEYID_BOOK_SYNC);
	return 0;
}
bookNode_t * get_bookNode(void){
	return &bookData;
}

int32 migumusicInit(void)
{
	int32 ret = 0;
	char * memberPhone=NULL;
	json_object *data_root;
    memset(&g_migu_devinfo,0,sizeof(RequestMiguDevInfo));
	
	g_migu_devinfo.mac= get_BT_Mac_addr();
	gatPrintf( GAT_DEBUG,"migumusicInit, test mac:%s.\n", g_migu_devinfo.mac);
	if (g_migu_devinfo.mac != NULL)
	{
		g_migu_devinfo.smartDeviceId =getDeviceId(g_migu_devinfo.mac); 
		if(g_migu_devinfo.smartDeviceId == NULL)
		{
			gatPrintf( GAT_DEBUG,"get smart deviceId error.\n");
			return -1;
		}
	}
	else
	{
		gatPrintf( GAT_DEBUG,"get bt mac error.\n");
		return -1;
	}
	if (access(DATA_FILE, F_OK) == 0)
	{
		data_root = json_object_from_file(DATA_FILE);
		if(data_root != NULL)
		{
			memberPhone = json_get_string_by_key(data_root,"memberPhone");
			if(memberPhone != NULL &&
				strlen(memberPhone) > 5 )
				g_migu_devinfo.msisdn = memberPhone;
			else
				g_migu_devinfo.msisdn = NULL;
			json_object_put(data_root);
		}
		else{
			g_migu_devinfo.msisdn = NULL;
		}
	}
	else
	{
		g_migu_devinfo.msisdn = NULL;
	}
	g_migu_devinfo.uid= NULL;
	g_migu_devinfo.sim= NULL;
			
	printf("%s:%d\n",__func__,__LINE__);
	return 0;
}

extern uint8 sys_volume_tab[];
extern int volueIndex;

int media_service_hander(struct json_object * json_root,uint32 mediaType)
{
	json_object * mediaInfo;
	mediaNode_t * p_mediaNode = &mediaNode;
	int32 type;
	char* media_URL; 

	json_pointer_get(json_root,"/data/result/data/result", &mediaInfo);
	if (0 == json_object_is_type(mediaInfo, json_type_array)) {
		gatPrintf( GAT_DEBUG,"get no newsInfo array.\n");
		//type = TYPE_RESULT;
		//sendInterruptEvt(type);
		return -1;
	} else 
		gatPrintf( GAT_DEBUG,"get mediaInfo array is right.\n");

	if (g_play_list_info.json_play_list != NULL)
	{
		json_object_put(g_play_list_info.json_play_list);
	}
	g_play_list_info.json_play_list = mediaInfo;
	g_play_list_info.playIndex = 0;
	g_play_list_info.mediaType = mediaType;
	g_play_list_info.list_len= json_object_array_length(mediaInfo);

	struct json_object *obj = json_object_array_get_idx(
								g_play_list_info.json_play_list,
								g_play_list_info.playIndex );
	if (obj == NULL)
	{
		 gatPrintf( GAT_ERROR,"%s: get play url error.\n",__func__);
		 return -2;
	}
	switch(mediaType)
	{
		case MEDIA_TYPE_NEWS:
		case MEDIA_TYPE_STORYTELLING:
		case MEDIA_TYPE_HEALTH:
		case MEDIA_TYPE_RADIO:
			media_URL = get_string_by_path(obj, "/url");
			break;

		case MEDIA_TYPE_JOKE:
			media_URL = get_string_by_path(obj, "/mp3Url");
			break;
		default:
			media_URL = get_string_by_path(obj, "/url");
		break;
	}
	if (media_URL != NULL) {
		p_mediaNode->play_url = media_URL;
		p_mediaNode->sourceType = mediaType;
		gatPrintf( GAT_DEBUG,"%s: get play url:%s.\n",__func__,media_URL);
		statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);
	}
	else {
		 //type = TYPE_RESULT;
		 //sendInterruptEvt(type);
		 gatPrintf( GAT_ERROR,"%s: get play url error.\n",__func__);
		 return -2;  
	}
	return 0;
}
int service_joke_hander(struct json_object * json_root)
{
	return 	media_service_hander(json_root,MEDIA_TYPE_JOKE);
}

int service_news_hander(struct json_object * json_root)
{
	return 	media_service_hander(json_root,MEDIA_TYPE_NEWS);
}

int service_storyTelling_hander(struct json_object * json_root)
{
	return 	media_service_hander(json_root,MEDIA_TYPE_STORYTELLING);
}

int service_health_hander(struct json_object * json_root)
{
	return 	media_service_hander(json_root,MEDIA_TYPE_HEALTH);
}

int service_radio_hander(struct json_object * json_root)
{
	return 	media_service_hander(json_root,MEDIA_TYPE_RADIO);
}

int service_bye_hander(struct json_object * json_root)
{

	int32 type;
	int32 evt;
		
	type = SOURCE_TYPE_INSTRUCTION_PAUSE;
	evt = EVT_PLAYER_SWITCH;
	statusEventNotify(evt, &type);

	playermanager_play_text("好的主人，那奴婢先跪安了。");

	return 0;
}


int cloudphone_cmd_triger(int cmd)
{
	int count = 0;
	while (pthread_mutex_trylock(&cloudphone_mutex)) {
		jsonPrintf(GAT_DEBUG,"get phone lock error.\n");
		usleep(100 * 1000);
		if (count++ > 30) {
			jsonPrintf(GAT_DEBUG,"can't get phone lock.\n");
			return -1;
		}
	}
	if (cmd < 0 || cmd > PHONE_IDLE) {
		jsonPrintf(GAT_DEBUG,"%s: phone cmd %d error.\n", __func__, cmd);
		return -1;
	}
	jsonPrintf(GAT_DEBUG,"%s: phone cmd %d .\n", __func__, cmd);
	if (cmd == PHONE_INIT ||
		cmd == PHONE_BIND)
	{
		gatTimerAdd((gatTimer_st *)getGetPhoneListTimer(), "getPhoneListTimer", 3,
			    getPhoneListTimerCb,
			    (void *) (getMiguClientNode()->fd));
	}

	cloudphone_cmd = cmd;

	pthread_cond_signal(&cloudphone_condition);
	pthread_mutex_unlock(&cloudphone_mutex);
	return 0;

}


int service_scheduleX_hander(struct json_object * json_root)
{
	json_object  *slots,*jo2;
	timerNode_t* p_timerNode = &timerData;
	char * intent = NULL;
	char * answerText=NULL;
	char * suggestDatetime = NULL;
	char * content = NULL;
	char * datetime = NULL;
	char * schedule_name = NULL;
	char tmp_path[64]={0};

	alarm_OK = 1;
	intent= get_string_by_path(json_root,"/data/result/semantic/0/intent");
	if(intent == NULL)
	{
		playermanager_play_text("哎呀，没听清，请重新设置吧");
		gatPrintf( GAT_DEBUG,"get intent is NULL!\n");
		return -1;
	}

	if (!strcmp(intent, "CREATE")){
		json_pointer_get(json_root,"/data/result/semantic/0/slots",&slots);
		if (slots == NULL)
		{
			playermanager_play_text("哎呀，没听清，请重新设置吧");
			gatPrintf( GAT_DEBUG,"get slots is NULL!\n");
			return -1;
		}
		if (json_object_is_type(slots,json_type_array))
		{
			int slots_len = json_object_array_length(slots);
			char * slots_name = NULL;
			int i;
			for (i = 0;i < slots_len;i++)
			{
				memset(tmp_path,0,64);
				sprintf(tmp_path,"/%d/name",i);
				slots_name = get_string_by_path(slots,tmp_path);
				printf("%s:%d i=%d get slots_name: %s.\n",__func__,__LINE__,i,slots_name);

				memset(tmp_path,0,64);
				sprintf(tmp_path,"/%d/value",i);

				if (strcmp(slots_name,"datetime") == 0 )
				{
					datetime = get_string_by_path(slots,tmp_path);

					memset(tmp_path,0,64);
					sprintf(tmp_path,"/%d/normValue/suggestDatetime",i);
					suggestDatetime = get_string_by_path(slots,tmp_path);
				}
				else if (strcmp(slots_name,"name") == 0 )
				{
					schedule_name = get_string_by_path(slots,tmp_path);		
				}
				else if (strcmp(slots_name,"content") == 0 )
				{
					content = get_string_by_path(slots,tmp_path);		
				}
				else
				{
					printf("%s: slots name not defined !\n",__func__);
				}
			}

			printf("####################################\n");
			gatPrintf( GAT_DEBUG,"schedule_name: %s\n",schedule_name);
			gatPrintf( GAT_DEBUG,"content: %s\n",content);
			gatPrintf( GAT_DEBUG,"datetime: %s\n",datetime );
			gatPrintf( GAT_DEBUG,"suggestDatetime:%s\n", suggestDatetime);
			printf("####################################\n");

			if (schedule_name != NULL && suggestDatetime != NULL)
			{
				if (!strcmp(schedule_name , "clock")) {

					gatPrintf( GAT_DEBUG,"this is clock setting.\n");
					iofLock(&p_timerNode->timerLock);
					sprintf(p_timerNode->intent, "%s", "setupClock");
					p_timerNode->timer_name = NULL;
					iofUnlock(&p_timerNode->timerLock);

				}
				else if (!strcmp(schedule_name , "reminder")) {
					gatPrintf( GAT_DEBUG,"this is reminder setting.\n");
					iofLock(&p_timerNode->timerLock);
					sprintf(p_timerNode->intent, "%s", "setupReminder");
					if (content != NULL)
					{
						p_timerNode->timer_name = content;
					}
					else
					{
						p_timerNode->timer_name = "提醒";
					}
					iofUnlock(&p_timerNode->timerLock);
				}
				p_timerNode->timerSetupTime = suggestDatetime;
				statusEventNotify(EVT_TIMER_ACTION, (timerNode_t*)&p_timerNode);
			}
			else
			{
				playermanager_play_text("哎呀，没听清，请重新设置吧");
				gatPrintf( GAT_DEBUG,"get timer params not enough !\n");
				return -1;
			}
		}
		else
		{
			playermanager_play_text("哎呀，没听清，请重新设置吧");
			gatPrintf( GAT_DEBUG,"get timer slots is not array type!\n");
			return -1;
		}

	}
	else if (!strcmp(intent, "CANCEL"))
	{
		json_pointer_get(json_root,"/data/result/semantic/0/slots",&slots);
		if (slots == NULL)
		{
			playermanager_play_text("哎呀，没听清，请重新设置吧");
			gatPrintf( GAT_DEBUG,"get slots is NULL!\n");
			return -1;
		}
		if (json_object_is_type(slots,json_type_array))
		{
			int slots_len = json_object_array_length(slots);
			char * slots_name = NULL;
			int i;
			for (i = 0;i < slots_len;i++)
			{
				memset(tmp_path,0,64);
				sprintf(tmp_path,"/%d/name",i);
				slots_name = get_string_by_path(slots,tmp_path);
				printf("%s:%d i=%d get slots_name: %s.\n",__func__,__LINE__,i,slots_name);

				memset(tmp_path,0,64);
				sprintf(tmp_path,"/%d/value",i);

				if (strcmp(slots_name,"datetime") == 0 )
				{
					datetime = get_string_by_path(slots,tmp_path);

					memset(tmp_path,0,64);
					sprintf(tmp_path,"/%d/normValue/suggestDatetime",i);
					suggestDatetime = get_string_by_path(slots,tmp_path);
				}
				else if (strcmp(slots_name,"name") == 0 )
				{
					schedule_name = get_string_by_path(slots,tmp_path);		
				}
				else if (strcmp(slots_name,"content") == 0 )
				{
					content = get_string_by_path(slots,tmp_path);		
				}
				else
				{
					printf("%s: slots name not defined !\n",__func__);
				}
			}
			printf("####################################\n");
			gatPrintf( GAT_DEBUG,"schedule_name: %s\n",schedule_name);
			gatPrintf( GAT_DEBUG,"content: %s\n",content);
			gatPrintf( GAT_DEBUG,"datetime: %s\n",datetime );
			gatPrintf( GAT_DEBUG,"suggestDatetime:%s\n", suggestDatetime);
			printf("####################################\n");

			if (schedule_name != NULL)
			{
				sprintf(p_timerNode->intent, "%s", "cancel");
				iofLock(&p_timerNode->timerLock);
				p_timerNode->timerSetupTime = NULL;
				p_timerNode->timer_name = schedule_name ;
				iofUnlock(&p_timerNode->timerLock);
				statusEventNotify(EVT_TIMER_ACTION, (timerNode_t*)&p_timerNode);
			}
		}
	}
	answerText = get_string_by_path(json_root,"/data/answerText");
	usleep(100*1000);
	if(answerText != NULL && alarm_OK==1){
		playermanager_play_text(answerText);
	}
	return 0;
}

int play_wordcup_music()
{
	json_object * json_root=NULL;
	json_object * musicInfos= NULL;
	json_object * m_musicInfos = NULL;
	json_object *ai_result= NULL;
	songNode_t* p_songNode = &songData;
	aiMessage_t* p_aiMessage = &aiMessage;
	int32 type;
	int ret;
	char * play_URL;
	char * song_name;
	char * song_artist;
	char * playid;
	char * musicName;
	char * singerName;
	char  volume[5];
	char tmpBuf[1024];

	json_root = json_object_from_file("/media/musicInfo.json");
	if (json_root == NULL )
	{
		printf("%s:%d: convert  migu_databu to json_root error.\n",__func__,__LINE__);
		return -1;
	}

	printf("%s:%d: in data: \n%s\n",__func__,__LINE__,
									json_object_to_json_string_ext(json_root,2));
	json_pointer_get(json_root, "/musicInfo", &musicInfos);
	if (! json_object_is_type(musicInfos,json_type_array))
	{
		printf("%s:%d: get musicInfos from json error.\n",__func__,__LINE__);
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}
	// 要播整个歌单。将歌单json信息添加到全局g_play_list_info
	{
		if (g_play_list_info.json_play_list != NULL)
		{
			json_object_put(g_play_list_info.json_play_list);
		}
		g_play_list_info.json_play_list = musicInfos;
		g_play_list_info.list_len= json_object_array_length(musicInfos);
		g_play_list_info.playMode=LIST;
		g_play_list_info.playIndex= 0;
		
	}

	p_songNode = update_songNode_by_index_from_list(musicInfos,g_play_list_info.playIndex);
	if (p_songNode == NULL)
	{
		playermanager_play_text("查询歌曲信息失败");
		return -1;
	}

	if(strcmp(p_songNode,"0")==0) 
	{
		memset(tmpBuf,0,1024);
		sprintf(tmpBuf, "%s", "咪咕音乐正在努力引入版权");
		playermanager_play_text(tmpBuf);
		//update aiMessage
		sprintf(p_aiMessage->serviceCode,"%s","musicX");
		ai_result = json_object_new_object();
		if(ai_result ==NULL)
		{
			return -1;
		}
		playid = get_string_by_path(m_musicInfos,"/musicId");
		musicName= get_string_by_path(m_musicInfos,"/musicName");
		singerName= get_string_by_path(m_musicInfos,"/singerName");
		json_object_object_add(ai_result, "musicId", json_object_new_string(playid));
		json_object_object_add(ai_result, "musicSource", json_object_new_int(1));
		json_object_object_add(ai_result, "isCpAuth", json_object_new_int(0));
		json_object_object_add(ai_result, "musicName", json_object_new_string(musicName));
		json_object_object_add(ai_result, "singerName", json_object_new_string(singerName));

		gatPrintf( GAT_DEBUG,"ai_result:%s\n",json_object_to_json_string_ext(ai_result,2));
		p_aiMessage->result = ai_result;
		memset(p_aiMessage->answerText,0,256);
		sprintf(p_aiMessage->answerText,"%s",tmpBuf);
		ret = package_json_pushaiMessage_action();
		printf("%s:%d pushaiMessage return: %d\n", __func__,__LINE__,ret);
		return -1;
	}

	if(	p_songNode->song_name !=NULL &&  
		p_songNode->singerName !=NULL && 
		p_songNode->song_url !=NULL)
	{

		memset(tmpBuf,0,1024);
		sprintf(tmpBuf, "%s%s", "好的马上为您播放世界杯主题曲",
								p_songNode-> singerName);
		statusEventNotify(EVT_MUSICPLAYER_ACTION, (songNode_t*)&p_songNode);

		playermanager_play_text(tmpBuf);
		printf("%s\n", tmpBuf);

		//update aiMessage
		sprintf(p_aiMessage->serviceCode,"%s","musicX");
		ai_result = json_object_new_object();
		if(ai_result ==NULL)
		{
			return -1;
		}
		json_object_object_add(ai_result, "musicId", json_object_new_string(p_songNode->playid));
		json_object_object_add(ai_result, "musicSource", json_object_new_int(1));
		json_object_object_add(ai_result, "isCpAuth", json_object_new_int(1));
		json_object_object_add(ai_result, "musicName", json_object_new_string(p_songNode->song_name));
		json_object_object_add(ai_result, "singerName", json_object_new_string(p_songNode->singerName ));

		p_aiMessage->result = ai_result;
		memset(p_aiMessage->answerText,0,256);
		sprintf(p_aiMessage->answerText,"%s",tmpBuf);
		ret = package_json_pushaiMessage_action();
		printf("%s:%d pushaiMessage return: %d\n", __func__,__LINE__,ret);
	} else {
		gatPrintf( GAT_DEBUG,"play_URL/song_name/song_artist is NULL!!!!!\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);
		return -1;
	}
	
	return 0;

}

char * _find_tagID_by_tag(json_object * tag_root,char * tag_name)
{
	json_object * group = NULL;
	json_object * groupInfos= NULL;
	json_object * tagInfos= NULL;
	char *tmp_buf;
	char *result=NULL;
	char json_path[40];
	int ret;
	int group_size ;
	int group_count ;
	int tag_size;
	int tag_count;

	char *resCode; 

	printf("%s: tag_name: %s\n",__func__,tag_name);
	
	//gatPrintf( GAT_DEBUG,"tag_root: \n%s\n",json_object_to_json_string_ext(tag_root,2));

	resCode = get_string_by_path(tag_root ,"/resCode");

	if(strcmp(resCode,"000000") != 0)
		return NULL;
	
	// get number of groups	
	json_pointer_get(tag_root , "/groupInfos", &groupInfos);
	group_size = json_object_array_length(groupInfos);
	if (group_size < 0)
		return NULL;

	//find group
	for (group_count =0;group_count <group_size;group_count++)
	{
		memset(json_path,0,40);
		sprintf(json_path,"/%d/tagInfos",group_count);
		//tmp_buf = get_string_by_path(tag_root ,"/groupInfos");

		// get number of tags in one group 
		json_pointer_get(groupInfos, json_path , &tagInfos);
		tag_size = json_object_array_length(tagInfos);

		//find tagId
		for(tag_count = 0;tag_count < tag_size;tag_count++)
		{
			memset(json_path,0,40);
			sprintf(json_path,"/%d/tagName",tag_count);
			tmp_buf = get_string_by_path(tagInfos, json_path);
			//printf("%s--%s\n",tmp_buf,tag_name);
			// get tagId
			if(strcmp(tmp_buf ,tag_name) == 0)
			{
				memset(json_path,0,40);
				sprintf(json_path,"/%d/tagId",tag_count);
				tmp_buf = get_string_by_path(tagInfos, json_path);
				if (tmp_buf == NULL )
				{
					printf("%s: line:%d get tagId failed.\n",__func__,__LINE__);
					return NULL;
				}

				result = malloc(strlen(tmp_buf));
				if (result == NULL)
				{
					printf("%s: line:%d malloc failed.\n",__func__,__LINE__);
					return NULL;
				}

				sprintf(result,"%s",tmp_buf);
				break;
			}
		}
		if (result != NULL)
			break;
	}
	
	return result;
}
char * find_tagID_by_tag( char * tag_name)
{
	json_object  * tag_root = NULL;
	char *result=NULL;
	int ret;

	printf("%s: tag_name: %s\n",__func__,tag_name);
	if (access(TAG_NAME_FILE, F_OK) == 0)
	{
		tag_root = json_object_from_file(TAG_NAME_FILE);
		result = _find_tagID_by_tag(tag_root,tag_name);
		json_object_put(tag_root); 
	}
	else
	{
		char * tag_buf[2*1024]={0};
		get_musicInfo_tag( tag_buf );
		if (tag_buf == NULL)
		{
			printf("%s: line:%d\n",__func__,__LINE__);
			return NULL;
		}

		tag_root = json_tokener_parse(tag_buf); 
		if( tag_root == NULL)
		{
			printf("%s: line:%d\n",__func__,__LINE__);
			return NULL;
		}
		result = _find_tagID_by_tag(tag_root,tag_name);
		ret = json_object_to_file_ext(TAG_NAME_FILE,tag_root ,2);
		json_object_put(tag_root); 
	}
	return result;
}

json_object * get_migu_buf_by_key(char * migu_key)
{
	json_object *json_migu_data;
	char * migu_musicId;

	printf("%s:migu_key= %s\n",__func__,migu_key);
	if (migu_key == NULL)
	{
		printf("migu key is NULL.\n");
		return NULL;	
	}
	memset(migu_databuf,0,migu_buf_size);
	get_musicInfo_key(migu_key, migu_databuf);
	
	json_migu_data = json_tokener_parse(migu_databuf); 
	
	//get musicId
	migu_musicId = get_string_by_path(json_migu_data,"/musicInfos/0/musicId");
	if (migu_key == NULL)
	{
		printf("%s: get musicId is NULL.\n");
		return NULL;	
	}
	//
	return get_migu_buf_by_musicID(migu_musicId);
}

int service_musicX_hander(struct json_object * json_root)
{
	json_object * slots, *jo2, *jo3;
	int32 type;
	int32 evt;
	int semantic_len;
	int slots_len;
	int is_play_list;
	char* inputAsrText=NULL;
	char* intent = NULL;
	char* slots_name=NULL;
	char* song_name=NULL;
	char* artist_name=NULL;
	char* tags_name=NULL;
	char* sourceType = NULL;
	char* playmode = NULL;
	char* slots_value=NULL;
	char* source_name=NULL;
	songNode_t* p_songNode = NULL;
	aiMessage_t* p_aiMessage = &aiMessage;
	char  migu_key[128];
	char  tmp_path[128];
	char tmp_buf[100];
	char * answerText=NULL;

	if(NULL == migu_databuf )
	{
		migu_databuf = (char*)iofMalloc(migu_buf_size);
		if(NULL == migu_databuf)
		{
			miguPrintf( GAT_ERROR,"%s malloc size:%d fail\r\n",__FUNCTION__,migu_buf_size );
			return -1;
		}
	}
	else
	{
		memset(migu_databuf,0,migu_buf_size);
	}

	inputAsrText = get_string_by_path(json_root,"/data/inputAsrText");
	if (inputAsrText != NULL )
	{
		sprintf(p_aiMessage->inputAsrText,"%s",inputAsrText); 
	}
	json_pointer_get(json_root, "/data/result/semantic/slots", &slots);
	//jo2 = /data/result/semantic
	
	if (slots== NULL)
	{
		gatPrintf( GAT_DEBUG,"get song name error.\n");
		return -1;
	}
	intent = get_string_by_path(json_root,"/data/result/semantic/intent");
	if (intent == NULL)
	{
		gatPrintf( GAT_DEBUG,"get intent error.\n");
		return -1;
	}

	if ( !json_object_is_type(slots,json_type_array)) 
	{
		gatPrintf( GAT_DEBUG,"slots type error.\n");
		return -1;
	}
	if (strcmp(intent,"play") == 0 || strcmp(intent,"PLAY") == 0 )
	{
		slots_len = json_object_array_length(slots);
		// play sond list
		if (slots_len > 1)
		{
			g_play_list_info.json_play_list= slots;
			g_play_list_info.playMode = RANDOM ;
			g_play_list_info.list_len= slots_len;
			
			g_play_list_info.playIndex = get_index_byMode_from_list(
					g_play_list_info.playMode,
					g_play_list_info.list_len,
					0);
			printf("%s: slots len=%d, play list\n",__func__,slots_len);
			p_songNode = update_songNode_by_index_from_list(g_play_list_info.json_play_list,g_play_list_info.playIndex);
			if (p_songNode == NULL)
			{
				playermanager_play_text("查询歌曲信息失败");
				return -1;
			}
			play_songNode(p_songNode,1);
		}
		// play one song
		else if (slots_len == 1)
		{
			json_object *json_migu_data;
			artist_name = get_string_by_path(slots,"/0/artist");
			song_name = get_string_by_path(slots,"/0/song");
			tags_name = get_string_by_path(slots,"/0/musicTag");

			//play single song
			if (artist_name != NULL && song_name != NULL )
			{
				memset(tmp_path,0,128);
				sprintf(tmp_path,"%s%s",artist_name,song_name );
				printf("%s:miguKey:%s\n",__func__,tmp_path);
				json_migu_data = get_migu_buf_by_key((char *)tmp_path);
				p_songNode = update_songNode_by_migudata(json_migu_data);
				if (p_songNode == NULL)
				{
					playermanager_play_text("查询歌曲信息失败");
					return -1;
				}
				play_songNode(p_songNode,1);
			}
			// play list
			else if (artist_name != NULL)
			{
				printf("%s:miguKey:%s\n",__func__,artist_name);
				get_musicInfo_key(artist_name, migu_databuf);
				parse_migu_buf(migu_databuf,1);
				return 0;
			}
			//play single song
			else if (song_name != NULL)
			{
				printf("%s:miguKey:%s\n",__func__,song_name );
				get_musicInfo_key(song_name , migu_databuf);
				parse_migu_buf(migu_databuf,0);
				return 0;
			}
			// search by tag
			else if (tags_name != NULL)
			{
				printf("%s:%d get tags_name: %s.\n",__func__,__LINE__,tags_name);
				if (strcmp (tags_name,"世界杯") == 0 )
				{
					play_wordcup_music();
					return 0;
				}
				else
				{
					char * tagId = NULL;
					sprintf(migu_key,"%s",tags_name);
					tagId = find_tagID_by_tag(tags_name);
					if (tagId != NULL)
					{
						get_musicInfo_musicTag(tagId, 0, 20, migu_databuf);
						free(tagId);
						is_play_list = 1;
						parse_migu_buf(migu_databuf,is_play_list);
						return 0;
					}
					else
					{
						playermanager_play_text("抱歉，我还不会按照此条件搜索歌曲，等我学会了，第一时间告诉你");
						return -1;
					}
				}
			}
			else
			{
				printf("%s: get no keys.\n",__func__);
				return -1;
			}

		}
		else
		{
			printf("%s:%d  parse slots error.\n",__func__,__LINE__);
			return -1;
		}
	}
	else if (strcmp(intent, "INSTRUCTION")==0 )
	{
		printf("%s:%d this is a INSTRUCTION request.\n",__func__,__LINE__);
		char * insType =get_string_by_path(slots,"/0/insType"); 
		if (insType == NULL) {
		printf("%s:%d: get insType value error.\n",__func__,__LINE__);
		return -1;
		} 
		// TODO
		if (strcmp(insType , "volume_select")==0 )
		{
		char* volume = get_string_by_path(slots,"/slots/1/value");
		if (volume == NULL) {
			gatPrintf( GAT_DEBUG,"get volume value error.\n");
			//return -1;
		} 
		else 
		{
			int volumeLevel = atoi(volume);
			int ret;
			char buf[1024];

			if ((volumeLevel < 0)||(volumeLevel>100)){
				memset(tmp_buf,0,100);
				sprintf(tmp_buf,"抱歉，音量的范围是百分之0,到百分之100,请重新设置");
				playermanager_play_text(tmp_buf);
				return -1;
			}

			printf("%s:%d set volume to [%s][%d].\n",__func__,__LINE__, volume, sys_volume_tab[volumeLevel/10]);
			sprintf(buf,"/etc/adckey/adckey_function.sh homeServerVolume %d", sys_volume_tab[volumeLevel/10]);
			system(buf);
			volueIndex = volumeLevel/10;
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"音量已调到百分之%s",volume);
			playermanager_play_text(tmp_buf);
			//return 0;
		}
		return 0;
		}
		else if (!strcmp(insType, "volume_plus"))
		{
			if(volueIndex<10)
				volueIndex++;
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"/etc/adckey/adckey_function.sh homeServerVolume %d",sys_volume_tab[volueIndex]);
			system(tmp_buf);
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"音量已调到百分之%d",volueIndex*10);
			playermanager_play_text(tmp_buf);
			return 0;
		}
		else if (!strcmp(insType, "volume_minus"))
		{
			if(volueIndex>0)
				volueIndex--;
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"/etc/adckey/adckey_function.sh homeServerVolume %d",sys_volume_tab[volueIndex]);
			system(tmp_buf);
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"音量已调到百分之%d",volueIndex*10);
			playermanager_play_text(tmp_buf);

		return 0;
		}
		else if (!strcmp(insType, "volume_max"))
		{
			volueIndex= 10;
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"/etc/adckey/adckey_function.sh homeServerVolume %d",sys_volume_tab[volueIndex]);
			system(tmp_buf);
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"音量已调到百分之%d",volueIndex*10);
			playermanager_play_text(tmp_buf);
			return 0;
		}
		else if (!strcmp(insType, "volume_min"))
		{
			volueIndex= 0;
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"/etc/adckey/adckey_function.sh homeServerVolume %d",sys_volume_tab[volueIndex]);
			system(tmp_buf);
			memset(tmp_buf,0,100);
			sprintf(tmp_buf,"音量已调到百分之%d",volueIndex*10);
			playermanager_play_text(tmp_buf);
			return 0;
		}
		else if (strcmp(insType , "next")==0 )
		{
			printf("------------ next ----------\n");
			printf("%s, %d,next\n", __FUNCTION__,__LINE__);
			type = SOURCE_TYPE_INSTRUCTION_NEXT;
			evt = EVT_PLAYER_SWITCH;
			statusEventNotify(evt, &type);
		}
		else if (strcmp(insType , "past")==0 )
		{
			printf("------------ prev ----------\n");
			printf("%s, %d,past\n", __FUNCTION__,__LINE__);
			type = SOURCE_TYPE_INSTRUCTION_PRE;
			evt = EVT_PLAYER_SWITCH;
			statusEventNotify(evt, &type);
		}
		else if (strcmp(insType , "pause")==0 ||
			 strcmp(insType , "mute")==0)
		{
			printf("%s, %d,pause\n", __FUNCTION__,__LINE__);
			type = SOURCE_TYPE_INSTRUCTION_PAUSE;
			evt = EVT_PLAYER_SWITCH;
			statusEventNotify(evt, &type);
			
			answerText = get_string_by_path(json_root,"/data/answerText");
			if (answerText != NULL) {
				gatPrintf( GAT_DEBUG,"get pause/mute cmd!\n");
				int flag=6;
				if(getPlayType()==PLAYTYPE_POETRY)
				{
					flag=6;
				}
				else
				{
					flag=1;
				}
				playermanager_play_text(answerText);
			} 
			else 
				gatPrintf( GAT_DEBUG,"pause/mute answerText is NULL!\n");
		}
		else if (strcmp(insType , "replay")==0 )
		{
			printf("%s, %d,replay\n", __FUNCTION__,__LINE__);
			type = SOURCE_TYPE_INSTRUCTION_REPLAY;
			evt = EVT_PLAYER_SWITCH;
			statusEventNotify(evt, &type);
			return 0;
		}
		else if (strcmp(insType , "change")==0)
		{
			printf("------------ change ----------\n");
			printf("%s, %d,change\n", __FUNCTION__,__LINE__);
			type = SOURCE_TYPE_INSTRUCTION_NEXT;
			evt = EVT_PLAYER_SWITCH;
			statusEventNotify(evt, &type);
			return 0;
		}
		else if (strcmp(insType , "broadcast")==0 )
		{
			char answerText[256] = {0};
			uint32 status = getDevStatus();
			p_songNode = &songData;
			if( status & DEV_STATUS_PLAYMUSIC)
			{
				sprintf(answerText,"正在播放的音乐是%s的%s",p_songNode->singerName,p_songNode->song_name);
			}
			else if( status & DEV_STATUS_PLAYBOOK )
			{

				bookNode_t* p_bookNode = &bookData;
				sprintf(answerText,"正在播放的书籍是%s的%s",p_bookNode->contentName,p_bookNode->chapterName);
			}
			else
			{
				sprintf(answerText,"%s","抱歉，音箱没有正在播放的内容.");
			}
			playermanager_play_text(answerText);
			return 0;
		}
		else if (strcmp(insType , "cycle")==0 )
		{
			if (getDevStatus() & DEV_STATUS_PLAYMUSIC )
			{
				g_play_list_info.playMode= SINGLE;
				playermanager_play_text("好的，已设置播放模式为单曲循环。");
			}
			else
			{
				playermanager_play_text("抱歉，当前模式不支持此操作。");
			}
			return 0;
		}
		else if (strcmp(insType , "order")==0 )
		{
			if (getDevStatus() & DEV_STATUS_PLAYMUSIC )
			{
				g_play_list_info.playMode= SEQUENCE;
				playermanager_play_text("好的，已设置播放模式为顺序播放。");
			}
			else
			{
				playermanager_play_text("抱歉，当前模式不支持此操作。");
			}
		}
		else if (strcmp(insType , "random")==0 )
		{
			if (getDevStatus() & DEV_STATUS_PLAYMUSIC )
			{
				g_play_list_info.playMode= RANDOM;
				playermanager_play_text("好的，已设置播放模式为随机播放。");
			}
			else
			{
				playermanager_play_text("抱歉，当前模式不支持此操作。");
			}
		}
		else if (strcmp(insType , "loop")==0 )
		{
			if (getDevStatus() & DEV_STATUS_PLAYMUSIC )
			{
				g_play_list_info.playMode= LIST;
				playermanager_play_text("好的，已设置播放模式为列表循环。");
			}
			else
			{
				playermanager_play_text("抱歉，当前模式不支持此操作。");
			}
		}
		else 
		{
			playermanager_play_text("主人，这个我还不会，你可以试试其他功能哦。");
		}
		return 0;
	}
	else
	{
		answerText = get_string_by_path(json_root,"/data/answerText");
		if (answerText != NULL )
		{
			playermanager_play_text(answerText);
		}
		else
		{
			answer_no_result();
		}
	}
	return 0;
}
#if 0
	//gatPrintf( GAT_DEBUG,"semantic data :\n%s\n",json_object_to_json_string_ext(jo1,2));
	if (json_object_is_type(slots,json_type_array))
	{
		semantic_len = json_object_array_length(jo1);
		//fot (i = 0;i< semantic_len;i++)
		intent = get_string_by_path(jo1,"/0/intent");
		if (intent == NULL)
		{
			printf("%s:%d  get intent error.\n",__func__,__LINE__);
			return -1;
		}
		printf("%s:%d  get intent: %s.\n",__func__,__LINE__,intent);
		if (strcmp(intent,"PLAY") == 0)
		{
			type = SOURCE_TYPE_STOP_BLUETOOTHMUSIC;
			evt = EVT_PLAYER_SWITCH;
			statusEventNotify(evt, &type);

			json_pointer_get(jo1,"/0/slots",&jo2);
			if (jo2 == NULL)
			{
				printf("%s:%d  get slots error.\n",__func__,__LINE__);
				return -1;
			}
			if (json_object_is_type(jo2,json_type_array))
			{
				
				int slots_len = json_object_array_length(jo2);
				int i;
				//读完所有参数
				for(i= 0;i<slots_len;i++)
				{
					//get slots name, song,artist,genre
					memset(tmp_path,0,128);
					sprintf(tmp_path,"/%d/name",i);
					slots_name = get_string_by_path(jo2,tmp_path);
					printf("%s:%d i=%d get slots_name: %s.\n",__func__,__LINE__,i,slots_name);
	
					memset(tmp_path,0,128);
					sprintf(tmp_path,"/%d/value",i);
	
					// song
					if (strcmp(slots_name,"song") == 0 )
					{
						song_name = get_string_by_path(jo2,tmp_path);
					}
					else if (strcmp(slots_name,"artist") == 0 ||
							(strcmp(slots_name,"band") == 0))
					{
						artist_name = get_string_by_path(jo2,tmp_path);
					}
					else if (strcmp(slots_name,"source") == 0) {
						source_name = get_string_by_path(jo2, tmp_path);
					}
					else if (strcmp(slots_name,"tags") == 0 || 
							 strcmp(slots_name,"genre") ==0 ||
							 strcmp(slots_name,"lang") ==0  )
					{
						tags_name = get_string_by_path(jo2, tmp_path);
					}
					else 
					{
						splayer_play_text("抱歉，我还不会按照此条件搜索歌曲，等我学会了，第一时间告诉你", 1);
						return -1;
					}

				}
					
				printf("####################################\n");
				gatPrintf( GAT_DEBUG,"artist_name:%s\n",artist_name);
				gatPrintf( GAT_DEBUG,"song_name:%s\n",song_name);
				gatPrintf( GAT_DEBUG,"source_name:%s\n",source_name);
				gatPrintf( GAT_DEBUG,"tags_name:%s\n",tags_name);
				printf("####################################\n");
				// formate migu_key
				if (artist_name != NULL && song_name != NULL)
				{
					printf("%s:%d get slots_name: %s.\n",__func__,__LINE__,slots_name);
					sprintf(migu_key,"%s%s",artist_name,song_name); 
					is_play_list = 0;
				}
				else if (song_name != NULL) 
				{
					printf("%s:%d get slots_name: %s.\n",__func__,__LINE__,slots_name);
					sprintf(migu_key,"%s",song_name); 
					is_play_list = 0;
				}
				else if (artist_name != NULL) 
				{
					printf("%s:%d get slots_name: %s.\n",__func__,__LINE__,slots_name);
					sprintf(migu_key,"%s",artist_name); 
					is_play_list = 1;
				}
				else if (source_name != NULL)
				{
					printf("%s:%d get slots_name: %s.\n",__func__,__LINE__,slots_name);
					sprintf(migu_key,"%s",source_name); 
					is_play_list = 1;
				}
				else if (tags_name != NULL)
				{
					printf("%s:%d get slots_name: %s.\n",__func__,__LINE__,slots_name);
					if (strcmp (tags_name,"世界杯") == 0 )
					{
						printf("%s:%d get tag_name: %s.\n",__func__,__LINE__,tags_name);
						play_wordcup_music();
						return 0;
					}
					else
					{
						char * tagId = NULL;
						sprintf(migu_key,"%s",tags_name); 
						tagId = find_tagID_by_tag(tags_name);
						if (tagId != NULL)
						{
							get_musicInfo_musicTag(tagId, 0, 20, migu_databuf);
							free(tagId);
							is_play_list = 1;
							parse_migu_buf(migu_databuf,is_play_list);
							return 0;
						}
						else
						{
							splayer_play_text("抱歉，我还不会按照此条件搜索歌曲，等我学会了，第一时间告诉你", 1);
							return -1;
						}
					}
				}
			}
			else
			{
				printf("%s:%d slots type is not array.\n",__func__,__LINE__);
				type = TYPE_RESULT;
				sendInterruptEvt(type);
				return -1;
			}
	
			gatPrintf( GAT_DEBUG,"migu_key :%s\n",migu_key);
			get_musicInfo_key(migu_key, migu_databuf);
			//gatPrintf( GAT_DEBUG,"migu_databuf: \n%s\n",migu_databuf);
			parse_migu_buf(migu_databuf,is_play_list);
			
		}
		else if (strcmp(intent,"RANDOM_SEARCH") == 0)
		{
			
			type = SOURCE_TYPE_STOP_BLUETOOTHMUSIC;
			evt = EVT_PLAYER_SWITCH;
			statusEventNotify(evt, &type);

			sprintf(migu_key,"%s","1781"); 
			g_play_list_info.playMode= RANDOM;
			is_play_list = 1;
			gatPrintf( GAT_DEBUG,"migu_key :%s\n",migu_key);
			get_musicInfo_musicSheetId(migu_key, migu_databuf);
			//gatPrintf( GAT_DEBUG,"migu_databuf: \n%s\n",migu_databuf);
			parse_migu_buf(migu_databuf,is_play_list);
	
		}

		return 0;
	}
	//object semantic is not type of json_array.
	else
	{
		gatPrintf( GAT_DEBUG,"object semantic is not type of json_array.\n");
		return -1;
	}

}
#endif

int  homeDemoHander()
{
	struct json_object * demo_content;
	int timer;
	int index,i;
	char * answerText;
	char * cmd;
	char * tmp_path[64]={0};
	// check index.	
	index = g_homeDemo_list_info.demoIndex;
	if (index >= g_homeDemo_list_info.list_len)
	{
		printf("%s: %d\n",__func__,__LINE__);
		gatTimerDel(getHomeDemoTimer());
		return 0;
	}

	sprintf(tmp_path,"/%d",index);
	json_pointer_get(g_homeDemo_list_info.demo_list, tmp_path, &demo_content);
	if(demo_content == NULL)
	{
		printf("%s: %d\n",__func__,__LINE__);
		return 0;
	}

	answerText = get_string_by_path(demo_content,"/Text");
	cmd = get_string_by_path(demo_content,"/cmd");
	timer = get_int_by_path(demo_content,"/time");
	if (answerText == NULL || cmd == NULL )
	{
		printf("%s: %d\n",__func__,__LINE__);
		return 0;	
	}

	playermanager_play_text(answerText);
	//push semantic cmd.
	//if (strcmp(answerText,"finish")== 0)
	if (g_homeDemo_list_info.demoIndex == (g_homeDemo_list_info.list_len-1))
	{
		gatPrintf( GAT_DEBUG,"demo finish\n");
		led_all_off_mute();
	}
	else
	{
		miguSemantic(cmd,strlen(cmd));
	}
	
	//add timer for next.
	gatTimerAdd(getHomeDemoTimer(), "HomeDemoTimer",timer,homeDemoHander,NULL);

	//update index
	g_homeDemo_list_info.demoIndex++;
	return 0;

}

void answer_no_result()
{
	int index;
	static char  ack_file[64]= "/media/sound/no_answer0.pcm";
	
	playermanager_play_file(ack_file);

	memset(ack_file,0,64);
	srand((int)time(0));
	index =((int)rand())%9;
	sprintf(ack_file,"/media/sound/no_answer%d.pcm",index);
	return ;

}

static int8 dialogFlag = 0;

void setDialogFlag(int8 flag)
{
	dialogFlag=flag;
}
int8 getDialogFlag(void)
{
	return dialogFlag;
}
int semantic_hander(json_object * json_root)
{
	int ret = 0;
	int32 type;
	int32 evt;
	json_object * jo1, *jo2, *jo3;
	char * serviceName=NULL;
	char * answerText=NULL;
	char * intent = NULL;
 	int32 dialogVal = 0;
	char tmp_buf[100];
	char* artist_name=NULL;
	char* song_name=NULL;
	char* tags_name=NULL;
	int8 telFlag = 0;

	//char* intent_key = NULL;
	sqlite3 * db = NULL;
	char* sourceType = NULL;
	char* inputAsrText=NULL;
	char* playmode = NULL;
	char* slots_name=NULL;
	char* source_name=NULL;
	char* slots_value=NULL;
	int is_play_list;
	phoneNode_t* p_phoneNode = &phoneData;
	songNode_t* p_songNode = &songData;
	timerNode_t* p_timerNode = &timerData;
	aiMessage_t* p_aiMessage = &aiMessage;
	mediaNode_t * p_mediaNode = &mediaNode;
	char  migu_key[128];
	char  tmp_path[128];
	char tmpData[256]={0};
	int32 datLen=0;

	gatTimerDel(getRequestHandleTimer());
	memset(migu_key,0,128);

	serviceName = get_string_by_path(json_root,"/data/serviceCode");
	if(NULL != serviceName)
	{
		datLen = strlen("recvd from homeserver ");
		memcpy(tmpData,"recvd from homeserver ",datLen);
		memcpy(tmpData+datLen,serviceName,strlen(serviceName));
		if (0 == strcmp("musicX", serviceName))
		{
			gatPrintf( GAT_DEBUG,"this is a music request.\n");
			service_musicX_hander(json_root);
			return 0;
		
		}
		else if (0 == strcmp("internetRadio", serviceName))
		{
			gatPrintf( GAT_DEBUG,"this is a internetRadio request.\n");
			struct BookRequestInfo book_request_info;
			json_pointer_get(json_root, "/data/result", &jo2);
			if (jo2 != NULL)
			{
				inputAsrText = get_string_by_path(json_root,"/data/inputAsrText");
				if (inputAsrText != NULL )
				{
					sprintf(p_aiMessage->inputAsrText,"%s",inputAsrText); 
				}
				memset(p_aiMessage->serviceCode,0,32);
				sprintf(p_aiMessage->serviceCode,"%s","internetRadio");

				book_request_info.chapterId = get_string_by_path(json_root,"/data/result/chapterId");// debug textbook "451570402";
				if (book_request_info.chapterId == NULL) {
					gatPrintf( GAT_DEBUG,"book request chapterId is NULL!!!\n");
					return -1;
				}
			//	book_request_info.chapterQuality = get_string_by_path(json_root,"/data/result/chapterQuality");
		//		book_request_info.contentId = get_string_by_path(json_root,"/data/result/contentId");
		//		book_request_info.contentName = get_string_by_path(json_root,"/data/result/contentName");
				sprintf(bookData.announce,"%s","好的，请欣赏");
				request_book_info_by_Id(&book_request_info);
				return 0;
			}
			else
			{
				answerText = get_string_by_path(json_root,"/data/answerText");
				playermanager_play_text(answerText);
				return 0;
			}
		 }
		// service musicX	 
		else if (0 == strcmp("scheduleX", serviceName)) {
			service_scheduleX_hander(json_root);
			
			return 0;
		}//end off scheduleX
		else if (0 == strcmp("LINGXI2018.home_bye", serviceName)) {
			service_bye_hander(json_root);
			return 0;
		}//end off scheduleX
	
		else if(0 == strcmp("telephone", serviceName)) {
			telFlag = 1;
		   json_pointer_get(json_root, "/data/result", &jo2);
		   intent= get_string_by_path(jo2,"/semantic/0/intent");
		   if (!strcmp(intent, "DIAL")) {
			   char* telephone_name = get_string_by_path(jo2,"/semantic/0/slots/0/value");
			   if (telephone_name == NULL) {
				  gatPrintf( GAT_DEBUG,"get telephone user name value err!\n");
				  answerText = get_string_by_path(json_root,"/data/answerText");
				  if (answerText != NULL)
				  {
                    playermanager_play_text(answerText);
                  }
				  else
				  {
					type = TYPE_RESULT;
					sendInterruptEvt(type);
                  }
				 return -1;
			   } 
			   else 
			   {
				  char* pName = get_string_by_path(jo2,"/semantic/0/slots/0/name");
				  int result=-1;
				   if(!strcmp(pName, "code"))
				   {
					   gatPrintf( GAT_DEBUG,"find telephone code!\n");
					   char* pTelNo = get_string_by_path(jo2,"/semantic/0/slots/0/value");
					   //char* pTelNo = "15889629673";
					   gatPrintf( GAT_DEBUG,"pTelNo:%s, len:%d!\n", pTelNo, strlen(pTelNo));
					   if((pTelNo == NULL)||strlen(pTelNo)>17)
					   {
						   gatPrintf( GAT_DEBUG,"telephone numbers invalid!\n");
						   type = TYPE_RESULT;
						   sendInterruptEvt(type);
						   return -1;
					   }
					   memset(&phoneData, 0, sizeof(phoneNode_t));
					   memcpy(phoneData.user_phone_num, (char*)pTelNo, strlen(pTelNo));
					   gatPrintf( GAT_DEBUG,"telPhone:%s!\n", phoneData.user_phone_num);
						//if (getDevStatus() & DEV_STATUS_PHONE_OK)
						if(g_cloudphone_ok)
					   {
					   		result = update_recallCallbackNo_to_file(pTelNo,NULL,0);
							gatPrintf( GAT_DEBUG,"update_recallCallbackNo_to_file result:%d\n", result);
							statusEventNotify(EVT_PHONE_CALLING, (phoneNode_t*)&p_phoneNode);

							memset(tmp_buf,0,100);
							sprintf(tmp_buf,"好的，正在打电话给%s",phoneData.user_phone_num);
							playermanager_play_text(tmp_buf);
					   }
					   else
					   {
							printf("getDevStatus: 0x%lx\n",getDevStatus());
						 	playermanager_play_text("您还没有启动打电话功能，请在APP上启动通话服务后再试试吧。");
					   }
					   return 0;
				   }
					
				   if(SQLITE_OK != sqlite3_open("/data/migubook.db", &db)) {
					   gatPrintf( GAT_DEBUG,"open db file failed: %s\n", sqlite3_errmsg(db));
					   type = TYPE_RESULT;
					   sendInterruptEvt(type);
					   return -1;
				   }
				   memset(&phoneData, 0, sizeof(phoneNode_t));
				   search_contact_name(telephone_name, db, search_callback);
				   sqlite3_close(db);
				   //if (getDevStatus() & DEV_STATUS_PHONE_OK)
					if(g_cloudphone_ok)
				   {
						if(strlen(phoneData.contactId) < 1)
						{
							playermanager_play_text("抱歉，通讯录中没有找到这个人。你可以同步手机通讯录到音箱后再试，或直接说手机号。");
							return;
						}
						update_recallCallbackNo_to_file(telephone_name,NULL,1);
						statusEventNotify(EVT_PHONE_CALLING, (phoneNode_t*)&p_phoneNode);

						memset(tmp_buf,0,100);
						sprintf(tmp_buf,"好的，正在打电话给%s",phoneData.user_name);
						playermanager_play_text(tmp_buf);
				   }
					else
					{
						printf("getDevStatus: 0x%lx\n",getDevStatus());
						playermanager_play_text("您还没有启动打电话功能，请在APP上启动通话服务后再试试吧。");
					}
				   }
			   return 0;
		   }//end of DIAL
		   else if(!strcmp(intent, "INSTRUCTION"))
		   {
			   char* telAction = get_string_by_path(jo2,"/semantic/0/slots/0/value");
			   if(telAction == NULL) 
			   {
				  gatPrintf( GAT_DEBUG,"get telephone user name value err!\n");
				  answerText = get_string_by_path(json_root,"/data/answerText");
				  if (answerText != NULL)
				  {
                    playermanager_play_text(answerText);
                  }
				  else
				  {
					type = TYPE_RESULT;
					sendInterruptEvt(type);
                  }
				 return -1;
			   } 
			   else
			   {
				   if(!strcmp(telAction, "CALLBACK")||!strcmp(telAction, "REDIAL")) //get callback phone number to call
				   {
						  char numberName[64]={0};
						  char *pTelNo=&numberName[0];
						  int callType = get_recallCallback_number(telAction,pTelNo);
						  gatPrintf( GAT_DEBUG,"Find callType:%d,recall phone number/name:%s, len:%d!\n", callType, pTelNo, strlen(pTelNo));
						
						  if(strlen(pTelNo)==0||callType<0) //||strlen(pTelNo)>17
						  {
							 gatPrintf( GAT_DEBUG,"telephone numbers invalid!\n");

						 	 playermanager_play_text("抱歉，未找到通话记录，您可以直接说出要拨打的联系人或手机号。");
							 //type = TYPE_RESULT;
							 //sendInterruptEvt(type);
							 return -1;
						  }
						  if(callType == 0) //call by number
						  {
							  memset(&phoneData, 0, sizeof(phoneNode_t));
							  memcpy(phoneData.user_phone_num, (char*)pTelNo, strlen(pTelNo));
							  gatPrintf( GAT_DEBUG,"telPhone:%s!\n", phoneData.user_phone_num);
						//	if (getDevStatus() & DEV_STATUS_PHONE_OK)
							if(g_cloudphone_ok)
							  {
								   statusEventNotify(EVT_PHONE_CALLING, (phoneNode_t*)&p_phoneNode);
							  
								   memset(tmp_buf,0,100);
								   sprintf(tmp_buf,"好的，正在打电话给%s",phoneData.user_phone_num);
								   playermanager_play_text(tmp_buf);
							  }
							  else
							  {
								  printf("getDevStatus: 0x%lx\n",getDevStatus());
								   playermanager_play_text("您还没有启动打电话功能，请在APP上启动通话服务后再试试吧。");
							  }
						  }
						  else if(callType == 1) //call by name
						  {
						  
							  if(SQLITE_OK != sqlite3_open("/data/migubook.db", &db)) {
								  gatPrintf( GAT_DEBUG,"open db file failed: %s\n", sqlite3_errmsg(db));
								  type = TYPE_RESULT;
								  sendInterruptEvt(type);
								  return -1;
							  }
							  memset(&phoneData, 0, sizeof(phoneNode_t));
							  search_contact_name(pTelNo, db, search_callback);
							  sqlite3_close(db);
							//if(getDevStatus() & DEV_STATUS_PHONE_OK)
							if(g_cloudphone_ok)
							  {
								   if(strlen(phoneData.contactId) < 1)
								   {
									   playermanager_play_text("抱歉，通讯录中没有找到这个人。你可以同步手机通讯录到音箱后再试，或直接说手机号。");
									   return;
								   }
								   statusEventNotify(EVT_PHONE_CALLING, (phoneNode_t*)&p_phoneNode);
							  
								   memset(tmp_buf,0,100);
								   sprintf(tmp_buf,"好的，正在打电话给%s",phoneData.user_name);
								   playermanager_play_text(tmp_buf);
							  }
							  else
							  {
								  printf("getDevStatus: 0x%lx\n",getDevStatus());
							      playermanager_play_text("您还没有启动打电话功能，请在APP上启动通话服务后再试试吧。");
							  }
						  }
						  else
						  {
							  gatPrintf( GAT_DEBUG,"callType invalid\n");
							  type = TYPE_RESULT;
							  sendInterruptEvt(type);
							  return -1;
						  }
						  return 0;
				   }
				   else if(!strcmp(telAction, "missed")) {
					    //
						//播报未接来电的号码需要添加
						//
						playermanager_play_text("主人，这个功能我还不会。");
					    /*播报完成就播放之前的内容*/
					    //type = TYPE_RESULT;
					//	sendInterruptEvt(type);
						return 0;
				   }
			   }
		   }//end of INSTRUCTION
		}//end of telephone
		else if (0 == strcmp("story", serviceName))
		{
		    char * play_URL = get_string_by_path(json_root,"/data/result/data/result/0/playUrl");
		    if (play_URL != NULL) {
		   	 p_mediaNode->play_url = play_URL;
		   	 p_mediaNode->sourceType = MEDIA_TYPE_STORY;
		   	 statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);
		    }
		    else
		    {
		   	playermanager_play_text("抱歉，我还没有这个资源。");
		   	//type = TYPE_RESULT;
		   	//sendInterruptEvt(type);
		   	 return -1;  
		    }
		}
		else if (0 == strcmp("joke", serviceName)) {
			ret = service_joke_hander(json_root);
			if (ret == -1) {
				answerText = get_string_by_path(json_root,"/data/answerText");
				if (answerText == NULL) {
				}
				else {
					playermanager_play_text(answerText);
					gatPrintf( GAT_DEBUG,"and!\n");
				}
				return -1;
			} else if (ret == -2){
				playermanager_play_text("抱歉，内容播放失败，试试其它功能吧。");
				gatPrintf( GAT_DEBUG,"json joke is error.!\n");
				return -1;
			}
		}
		else if (0 == strcmp("news", serviceName))
		{
			ret = service_news_hander(json_root);
			if (ret == -1) {
				answerText = get_string_by_path(json_root,"/data/answerText");
				if (answerText == NULL) {
				} else {
					playermanager_play_text(answerText);
					gatPrintf( GAT_DEBUG,"json news is err!\n");
				}
				return -1;
			} else if (ret == -2){

			}
		}
		else if (!strcmp("cmd", serviceName)) {
			mediaNode_t * p_mediaNode = &mediaNode;
			json_pointer_get(json_root, "/data/result", &jo2);

			intent= get_string_by_path(jo2,"/semantic/0/intent");
			if (!strcmp(intent, "INSTRUCTION")) 
			{
			char* cmd = get_string_by_path(jo2,"/semantic/0/slots/0/value");
			char* cmd_name = get_string_by_path(jo2,"/semantic/0/slots/0/name");
			if ( strcmp(cmd_name, "volume") == 0)
			{
				char * volume = get_string_by_path(jo2,"/semantic/0/slots/0/value");
				if (volume == NULL) {
					gatPrintf( GAT_DEBUG,"get volume value error.\n");
					//return -1;
				}
				else 
				{
					int volumeLevel = atoi(volume);
					int ret;
					char buf[1024];

					if ((volumeLevel < 0)||(volumeLevel>100)){
						memset(tmp_buf,0,100);
						sprintf(tmp_buf,"抱歉，音量的范围是百分之0,到百分之100,请重新设置");
						playermanager_play_text(tmp_buf);
						return -1;
					}

					printf("%s:%d set volume to [%s][%d].\n",__func__,__LINE__, volume, sys_volume_tab[volumeLevel/10]);
					sprintf(buf,"/etc/adckey/adckey_function.sh homeServerVolume %d", sys_volume_tab[volumeLevel/10]);	
					system(buf);
					volueIndex = volumeLevel/10;
					memset(tmp_buf,0,100);
					sprintf(tmp_buf,"音量已调到%s",volume);
					playermanager_play_text(tmp_buf);
					//return 0;
				}
				return 0;	
			}
			
			if ( strcmp(cmd, "next") == 0) {
				printf("------------ next ----------\n");
				printf("%s, %d,next\n", __FUNCTION__,__LINE__);
				type = SOURCE_TYPE_INSTRUCTION_NEXT;
				evt = EVT_PLAYER_SWITCH;
				statusEventNotify(evt, &type);
			}
			else if (!strcmp(cmd, "previous")) {
				printf("------------ prev ----------\n");
				printf("%s, %d,past\n", __FUNCTION__,__LINE__);
				type = SOURCE_TYPE_INSTRUCTION_PRE;
				evt = EVT_PLAYER_SWITCH;
				statusEventNotify(evt, &type);
			}
			else if (!strcmp(cmd, "change")) {
				printf("------------ change ----------\n");
				printf("%s, %d,change\n", __FUNCTION__,__LINE__);
				type = SOURCE_TYPE_INSTRUCTION_NEXT;
				evt = EVT_PLAYER_SWITCH;
				statusEventNotify(evt, &type);
			}
			else if (!strcmp(cmd, "pause")||(!strcmp(cmd, "mute"))) {

				int flag=6;
				printf("%s, %d,pause\n", __FUNCTION__,__LINE__);
				//type = SOURCE_TYPE_INSTRUCTION_PAUSE;
				//evt = EVT_PLAYER_SWITCH;
				//statusEventNotify(evt, &type);
				
				answerText = get_string_by_path(json_root,"/data/answerText");
				if (answerText != NULL) {
					gatPrintf( GAT_DEBUG,"get pause/mute cmd!\n");
					if(getPlayType()==PLAYTYPE_POETRY)
					{
						flag=6;
					}
					else
					{
						flag=8;
					}
					playermanager_play_text(answerText);
				}
				else 
				{
					playermanager_play_file("/media/sound/paused.pcm");
					gatPrintf( GAT_DEBUG,"pause/mute answerText is NULL!\n");
				}
			}
			else if (!strcmp(cmd, "continue")) {
				printf("%s, %d,continue\n", __FUNCTION__,__LINE__);
				type = SOURCE_TYPE_INSTRUCTION_REPLAY;
				evt = EVT_PLAYER_SWITCH;
				statusEventNotify(evt, &type);
			}
			else if (!strcmp(cmd, "openBluetooth")) {
				answerText = get_string_by_path(json_root,"/data/answerText");
				if (answerText != NULL) {
					gatPrintf( GAT_DEBUG,"Bluetooth open!\n");
					system("/etc/init.d/S44bluetooth start");
					playermanager_play_text(answerText);
				}
				else
					gatPrintf( GAT_DEBUG,"openBluetooth cmd is NULL!\n");
			}
			else if (!strcmp(cmd, "closeBluetooth")) {
				answerText = get_string_by_path(json_root,"/data/answerText");
				if (answerText != NULL) {
					gatPrintf( GAT_DEBUG,"Bluetooth close!\n");
					system("/etc/init.d/S44bluetooth stop");
					playermanager_play_text(answerText);
				}
				else
					gatPrintf( GAT_DEBUG,"closeBluetooth cmd is NULL!\n");
			}
			else if (strcmp(cmd, "volume_select") == 0)
			{
				printf("%s:%d \n",__func__,__LINE__);
				char* volume=NULL;
				volume = get_string_by_path(jo2,"/semantic/0/slots/1/value");
				if (volume == NULL) {
					gatPrintf( GAT_DEBUG,"get volume value error.\n");
					//return -1;
				}
				else 
				{
				printf("%s:%d \n",__func__,__LINE__);
					int volumeLevel = atoi(volume);
					int ret;
					char buf[1024];

					if ((volumeLevel < 0)||(volumeLevel>100)){
						memset(tmp_buf,0,100);
						sprintf(tmp_buf,"抱歉，音量的范围是百分之0,到百分之100,请重新设置");
						playermanager_play_text(tmp_buf);
						return -1;
					}

					printf("%s:%d \n",__func__,__LINE__);
					printf("%s:%d set volume to [%s][%d].\n",__func__,__LINE__, volume, sys_volume_tab[volumeLevel/10]);
					sprintf(buf,"/etc/adckey/adckey_function.sh homeServerVolume %d", sys_volume_tab[volumeLevel/10]);	
					system(buf);
					volueIndex = volumeLevel/10;
					memset(tmp_buf,0,100);
					sprintf(tmp_buf,"音量已调到百分之%s",volume);
					playermanager_play_text(tmp_buf);
					//return 0;
				}//end of else volume==NULL
			}//end of volume_select
			}//end of INSTRUCTION
			return 0;
		}//end of cmd
		else if (0 == strcmp("health", serviceName)) 
		{
#if 0
			 char * play_URL = get_string_by_path(json_root,"/data/result/data/result/0/url");
			 if (play_URL != NULL) {
				 p_mediaNode->play_url = play_URL;
				 p_mediaNode->sourceType = MEDIA_TYPE_HEALTH;
				 statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);
			 }
			 else
			 {
				 type = TYPE_RESULT;
				 sendInterruptEvt(type);
		 
				 return -1;  
			 }
#endif
			ret = service_health_hander(json_root);
			if (ret == -1) {
				answerText = get_string_by_path(json_root,"/data/answerText");
				if (answerText == NULL) {
				}
				else {
					playermanager_play_text(answerText);
					gatPrintf( GAT_DEBUG,"and!\n");
				}
				return -1;
			} else if (ret == -2){
				playermanager_play_text("抱歉，内容播放失败，试试其它功能吧。");
				gatPrintf( GAT_DEBUG,"json health is error.!\n");
				return -1;
			}
		 } else if (!strcmp("radio", serviceName)) {
#if 0
				char * play_URL = get_string_by_path(json_root,"/data/result/data/result/0/url");
				if (play_URL == NULL) {
					answerText = get_string_by_path(json_root,"/data/answerText");
					splayer_play_text(answerText,7);
					return -1;
				} else {
					p_mediaNode->play_url = play_URL;
					p_mediaNode->sourceType = MEDIA_TYPE_RADIO;
					statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);
				}
#endif
				ret = service_radio_hander(json_root);
				if (ret == -1) {
					answerText = get_string_by_path(json_root,"/data/answerText");
					if (answerText == NULL) {
					}
					else {
						playermanager_play_text(answerText);
						gatPrintf( GAT_DEBUG,"and!\n");
					}
					return -1;
				} else if (ret == -2){
					playermanager_play_text("抱歉，内容播放失败，试试其它功能吧。");
					gatPrintf( GAT_DEBUG,"json radio is error.!\n");
					return -1;
				}
			} else if (!strcmp("crossTalk", serviceName)) {
			char * play_URL = get_string_by_path(json_root,"/data/result/data/result/0/url");
				p_mediaNode->play_url = play_URL;
				p_mediaNode->sourceType = MEDIA_TYPE_CROSSTALK;
				statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);
		} else if (!strcmp("drama", serviceName)) {
			char * play_URL = get_string_by_path(json_root,"/data/result/data/result/0/url");
				p_mediaNode->play_url = play_URL;
				p_mediaNode->sourceType = MEDIA_TYPE_DRAMA;
			statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);
		} else if (!strcmp("animalCries", serviceName)) {
			char * play_URL = get_string_by_path(json_root,"/data/result/data/result/0/url");
				p_mediaNode->play_url = play_URL;
				p_mediaNode->sourceType = MEDIA_TYPE_ANIMALCRIES;
			statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);

		} else if (!strcmp("storyTelling", serviceName)) {
			ret = service_storyTelling_hander(json_root);
			if (ret == -1) {
				answerText = get_string_by_path(json_root,"/data/answerText");
				if (answerText == NULL) {
				} else {
					playermanager_play_text(answerText);
					gatPrintf( GAT_DEBUG,"json storyTelling is err!\n");
				}
				return -1;
			} else if (ret == -2){

			}
		} else if (!strcmp("history", serviceName)) {
			char * play_URL = get_string_by_path(json_root,"/data/result/data/result/0/url");
			p_mediaNode->play_url = play_URL;
			p_mediaNode->sourceType = MEDIA_TYPE_HISTORY;
			statusEventNotify(EVT_MEDIAPLAYER_ACTION, (mediaNode_t *)&p_mediaNode);
		}
		else if (0 == strcmp("translation", serviceName))
		{
		char * result_str = get_string_by_path(json_root,"/data/result/data/result/0/translated");
		char * original_str = get_string_by_path(json_root,"/data/result/data/result/0/original");
		char * transLangCountry= get_string_by_path(json_root,"/data/result/data/result/0/transLangCountry");
		char * say_to_en = "的英文翻译是";
		char * say_to_cn = "的中文意思是";
		char * say = "的翻译是";

		if (original_str != NULL && result_str != NULL && transLangCountry != NULL) {
				// say en
			 int totalLen=0;
			 //char answerText[512]={0};
			 totalLen = strlen(result_str)+strlen(original_str)+strlen(say_to_en)+4;

			 char * answerText = (int8*)iofMalloc(totalLen);	
			 if (answerText == NULL)
			 {
				 printf("%s:%d malloc answerText error.\n");
				 return -1;
			 }
			 memset(answerText,0, totalLen);
			 if (0 == strcmp("cn", transLangCountry))
			 {
				sprintf(answerText ,"%s%s%s",original_str,say_to_cn,result_str);
			 }
			 else if (0 == strcmp("en", transLangCountry))
			 {
				sprintf(answerText ,"%s%s%s",original_str,say_to_en,result_str);
			 }
			 else
			 {
				sprintf(answerText ,"%s%s%s",original_str,say,result_str);
			 }
			
			 if (answerText != NULL)
			 {
				gatPrintf( GAT_DEBUG,"result:%s\n",answerText);
				playermanager_play_text(answerText);
				free(answerText);
				answerText = NULL; 
				return 0;
			 }
			 else
			 {
				playermanager_play_text("抱歉，翻译失败");
				return -1;
			 }
		}
		else
		{
		 // say sorry
		 playermanager_play_text("抱歉，翻译失败");
		 return -1;  
		}
		}
		else if (0 == strcmp("stock", serviceName))
		{
			char * currentPrice;
			char * riseRate;
			char * stock_name;
			char answerText[64] = {0};
		    currentPrice = get_string_by_path(json_root,"/data/result/data/result/0/currentPrice");
		    riseRate = get_string_by_path(json_root,"/data/result/data/result/0/riseRate");
		    stock_name = get_string_by_path(json_root,"/data/result/data/result/0/name");
			if ((currentPrice != NULL)  && ( riseRate != NULL))
			{
				sprintf(answerText,"股票名称%s的当前价格是%s,涨跌幅%s",stock_name,currentPrice,riseRate);
				playermanager_play_text(answerText);
			}
			else
			{
				playermanager_play_text("抱歉，股票信息查询失败");
			}

			return 0;
		}
	
		else if (0 == strcmp("poetry", serviceName)) //δ????
		{
			#if 0
			int poetryArraySize = 0;
			json_object * poetryInfo;
			//char answerText[512] = {0};
			//char * answerText=NULL;

			json_pointer_get(json_root,"/data/result/data/result", &poetryInfo);
			if (0 == json_object_is_type(poetryInfo, json_type_array)) {
				gatPrintf( GAT_DEBUG,"get no poetry.\n");
				return -1;
			} else gatPrintf( GAT_DEBUG,"get poetryInfo array is right.\n");
			poetryArraySize = json_object_array_length(poetryInfo);
			if (poetryInfoObject != NULL) {
				poetryIndex = 0;
				json_object_put(poetryInfoObject);
				poetryInfoObject = NULL;
			}
			poetryInfoObject = json_object_new_object();
			if (NULL == poetryInfoObject) {
				gatPrintf( GAT_DEBUG,"new poetryInfoObject failed.\n");
				return -1;
			}
			
			struct json_object *obj = json_object_array_get_idx(poetryInfo, 0);
			json_object_object_add(poetryInfoObject, "poetryInfo", poetryInfo);
			poetryIndex = 0;
			setPoetryPlayingFlag(1);
			//setDevStatus(DEV_STATUS_POETRY, 1);
			answerText = get_string_by_path(json_root,"/data/answerText");
			if (strlen(answerText) > 2)
			{
				splayer_play_text(answerText,7);
			}
			else
			{
				answer_no_result();
			}
			
			return 0;
			#endif
		}
		else if (0 == strcmp("restaurantSearch", serviceName))
		{
			int restauranArraySize = 0;
			json_object * restaurantInfo;
			char answerText[512] = {0};
			json_pointer_get(json_root,"/data/result/data/result", &restaurantInfo);
			if (0 == json_object_is_type(restaurantInfo, json_type_array)) {
				gatPrintf( GAT_DEBUG,"get no restaurantInfo array.\n");
				return -1;
			} else gatPrintf( GAT_DEBUG,"get restaurantInfo array is right.\n");
			restauranArraySize = json_object_array_length(restaurantInfo);
			if (restaurantInfoObject != NULL) {
				restaurantIndex = 0;
				json_object_put(restaurantInfoObject);
				restaurantInfoObject = NULL;
			}
			restaurantInfoObject = json_object_new_object();
			if (NULL == restaurantInfoObject) {
				gatPrintf( GAT_DEBUG,"new restaurantInfoObject failed.\n");
				return -1;
			}
			
			struct json_object *obj1 = json_object_array_get_idx(restaurantInfo, 0);
			struct json_object *obj2 = json_object_array_get_idx(restaurantInfo, 1);

			json_object_object_add(restaurantInfoObject, "restaurantInfo", restaurantInfo);
			memset(answerText, '\0', sizeof(answerText));
			restaurantIndex += 2;
			sprintf(answerText,"已经为你查询到附近的%d家餐馆,比如%s%s%s,%s,和%s%s%s,%s等等", 
				restauranArraySize, 
				get_string_by_path(obj1,"/city"), get_string_by_path(obj1,"/area"), get_string_by_path(obj1,"/address"),
				get_string_by_path(obj1,"/name"), get_string_by_path(obj2,"/city"), get_string_by_path(obj2,"/area"),
				get_string_by_path(obj2,"/address"), get_string_by_path(obj2,"/name"));
			
			setDevStatus(DEV_STATUS_PLAYRESTAURANT, 1);
			playermanager_play_text(answerText);

			return 0;
#if 0
		} else if (!strcmp("riddle", serviceName)) {
			json_object * riddleInfo;
			json_pointer_get(json_root,"/data/result/data/result", &riddleInfo);
			if (0 == json_object_is_type(riddleInfo, json_type_array)) {
				gatPrintf( GAT_DEBUG,"get no riddleInfo array.\n");
				return -1;
			} else gatPrintf( GAT_DEBUG,"get riddleInfo array is right.\n");
			struct json_object *obj = json_object_array_get_idx(riddleInfo, 0);
			splayer_play_text(get_string_by_path(obj,"/title"),1);
			return 0;
#endif
		} else if (!strcmp("calc", serviceName))  {
			answerText = get_string_by_path(json_root,"/data/answerText");
			if (answerText != NULL) {
				gatPrintf( GAT_DEBUG,"get calc cmd!\n");
				playermanager_play_text(answerText);
			} else gatPrintf( GAT_DEBUG,"calc answerText is NULL!\n");
			return 0;	
		}
		else if (0==strcmp("weather", serviceName)||
				0==strcmp("AIUI.Riddle", serviceName)||
				0==strcmp("flight", serviceName)||
				0==strcmp("train", serviceName) 
				) {
			answerText = get_string_by_path(json_root,"/data/answerText");
			gatPrintf( GAT_DEBUG,"weather skill open duolun.\n");
			playermanager_play_text(answerText);
			return 0;
		}
		else 
		{
			 printf("%s type not supported!\n", serviceName);

			 answerText = get_string_by_path(json_root,"/data/answerText");

			 if (strlen(answerText) > 2)
			 {
			     playermanager_play_text(answerText);
			     if(getDialogFlag() == 1)
			     {
			   	  setDialogFlag(0);
			   	  //caeWakeupEnable(1);
			     }
			 }
			 else
			 {
			     answer_no_result();
			 }
			 return -1;
		}
		//if (0 == strcmp("weather", serviceName))
	}//end serviceCode != NULL

	if(isCloudPhoneBusy()||isBtPhoneBusy()){
		gatPrintf( GAT_DEBUG,"isPhoneBusy. cancel otaupdate\n");
		return 0;
	}

	answerText = get_string_by_path(json_root,"/data/answerText");
	
	if (answerText == NULL || strlen(answerText) < 2)
	{
	   answer_no_result();
	   return 0;
	}
	if (NULL!=serviceName && (
			0 == strcmp("story", serviceName)||
			0 == strcmp("news", serviceName)||
			0 == strcmp("joke", serviceName)||
			0 == strcmp("health", serviceName)||
			0 == strcmp("radio", serviceName)||
			0 == strcmp("crossTalk", serviceName)||
			0 == strcmp("drama", serviceName)||
			0 == strcmp("animalCries", serviceName)||
			0 == strcmp("storyTelling", serviceName)||
			0 == strcmp("history", serviceName)	))
	{
		//play url after speech
		playermanager_play_text(answerText);
	}
	else
	{
		playermanager_play_text(answerText);
	}
	return 0;
}


int getImsInfo_hander(json_object * json_root)
{
	printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));
	return 0;
}

int getImsPhoneList_hander(json_object * json_root)
{
	int ret=-1;
	ret=updateImsPhoneList(json_root);
	return ret;
}

static int updateImsPhoneList(json_object * json_root)
{
	json_object *jo1, *jo2;
	int arraySize = 0;
	int i;
	struct json_object *phonebook_object = NULL;
	char* nickname = NULL;
	char* number = NULL;
	char* status = NULL;
	struct json_object *obj;
	int tag = 0;
	sqlite3 * db = NULL;

	json_pointer_get(json_root, "/data", &jo1);
	json_object_object_get_ex(jo1, "persons", &jo2);
	if (0 == json_object_is_type(jo2, json_type_array)) {
		gatPrintf( GAT_DEBUG,"persons is`t array!\n");
		return -1;
	}
	arraySize = json_object_array_length(jo2);
	if (arraySize == 0)
	{
		gatPrintf( GAT_DEBUG,"persons contect length is 0!\n");
		//remove("/data/migubook.db");
		//return -1;
	}
	phonebook_object = json_object_new_object();
	if (NULL == phonebook_object) {
		json_object_put(phonebook_object);
		gatPrintf( GAT_DEBUG,"new json phonebook object failed.\n");
		return -1;
	}
	json_object_object_add(phonebook_object, "message", json_object_new_string("migu phonebook"));
	json_object_object_add(phonebook_object, "allRowCount", json_object_new_int(arraySize));
	json_object_object_add(phonebook_object, "persons", jo2);
	if (arraySize > 0)
	{
		//telNamePars(jo2, arraySize); //mark
	}
	
	if(SQLITE_OK != sqlite3_open("/data/migubook.db", &db)) {
		gatPrintf( GAT_DEBUG,"open db file failed: %s\n", sqlite3_errmsg(db));
		return -1;
	}
	create_table(db);
	update_contact((char*)json_object_to_json_string(phonebook_object), db);
	//search_contact_all(db, search_callback);
	//search_contact_name("renjun",db, search_callback);
	//search_contact_number("10010",db, search_callback);
	sqlite3_close(db);
	return 0;
}
int pushAiMessage_hander(json_object * json_root)
{
	//printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));
	return 0;
}
int getMdevUser_hander(json_object * json_root)
{
	printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));
	return 0;
}

int get_index_by_id_from_list(json_object *list, char * musicID)
{
	int index;
	int i;
	int list_len;
	char * tmp_id;
	char tmp_path[20];
	if (0 == json_object_is_type(list, json_type_array))
	{
		return -1;	
	}
	list_len =  json_object_array_length(list);
	for(i =0;i<list_len;i++)
	{
		memset(tmp_path,0,20);		
		sprintf(tmp_path,"/%d/musicId",i);
		tmp_id = get_string_by_path(list,tmp_path);	
		if(strcmp(tmp_id,musicID)== 0) 
			return i;
	}
	return -1;
}
int pushMusics_hander(json_object * json_root)
{
	int ret = 0;
	int list_len;
	int playMode;
	int index;
	int32 type;
	json_object * musicInfo, *jo2, *jo3;
	songNode_t* p_songNode = &songData;
	char *play_URL;
	char* musicId;
	char* song_name;
	char* p_fromPosition;
	char  volume[5];

	//printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));
//	jo1 = json_object_get_object();

	json_pointer_get(json_root,"/data/musicInfo", &musicInfo);
	if (0 == json_object_is_type(musicInfo, json_type_array))
	{
		gatPrintf( GAT_DEBUG,"get no musicInfo array.\n");	
		return -1;
	}
	list_len =  json_object_array_length(musicInfo);
	if (list_len == 1)
	{

		p_songNode = update_songNode_by_index_from_list(musicInfo,0);
		if (p_songNode == NULL )
		{
			printf("%s: get no musicInfo error.\n");	
			return -1;
		}

		if(p_songNode -> song_url != NULL)
		{
			p_songNode ->play_right_now = 1;
			statusEventNotify(EVT_MUSICPLAYER_ACTION, (songNode_t*)&p_songNode);
		}
		else
		{
			gatPrintf( GAT_DEBUG,"can't find this music.\n");	
			type = TYPE_RESULT;
			sendInterruptEvt(type);
		}		

	}
	else if(list_len > 1 )
	{
		//1. clear old list in g_play_list_info
		if (g_play_list_info.json_play_list != NULL)
		{
			json_object_put(g_play_list_info.json_play_list);	
			g_play_list_info.playIndex = 0;
		}

		//2. update g_play_list_info.json_play_list
		g_play_list_info.json_play_list = musicInfo;
		//3. set other params.
		g_play_list_info.list_len= list_len;

		playMode = get_int_by_path(json_root,"/data/playMode"); 
		if (playMode == -1)
		{
			playMode = RANDOM; 
		}
		g_play_list_info.playMode = playMode;  

		char * plyingMusicId = get_string_by_path(json_root, "/data/playingMusicId");
		// get play index
		if (plyingMusicId  != NULL)
		{
			//find index by id from list.
			index = get_index_by_id_from_list(g_play_list_info.json_play_list, plyingMusicId);
			if (index < 0) 
				index = 0;
			if (index > (g_play_list_info.list_len-1)) 
				index = g_play_list_info.list_len-1;

			// play by index.
			g_play_list_info.playIndex = index;
		}
		// get play index
		else
		{
			if(g_play_list_info.playMode == SEQUENCE |
				g_play_list_info.playMode == LIST)
			{
				g_play_list_info.playIndex = 0;
			}
			else
			{
				index = get_index_byMode_from_list(playMode,list_len,0);
				if (index > 0 && index < list_len)
				{
				   g_play_list_info.playIndex = index;
				}
				else
				{
				   g_play_list_info.playIndex = 0;
				}
			}

		}
		p_songNode = update_songNode_by_index_from_list(g_play_list_info.json_play_list,
													g_play_list_info.playIndex);
		if (p_songNode == NULL )
		{
			printf("%s: get no musicInfo error.\n");	
			return -1;
		}

		if(p_songNode -> song_url != NULL)
		{
			p_songNode->play_right_now = 1;
			statusEventNotify(EVT_MUSICPLAYER_ACTION, (songNode_t*)&p_songNode);
		}
		else
		{
			gatPrintf( GAT_DEBUG,"can't find this music.\n");	
			type = TYPE_RESULT;
			sendInterruptEvt(type);
		}

	}
	return 0;
}


int pushControl_hander(json_object * json_root)
{
	int32 type;
	int32 evt;
	json_object * jo1, *jo2, *jo3;
	int ret = 0;
	int Int_controlCode;
	int Int_fromUser;
	char buf[128]={0};
	char volume[5]={0};

	songNode_t* p_songNode = &songData;

#define VOL 10
#define START 11
#define PAUSE 12
#define NEXT 13
#define PRE 14
#define PLAYMODE_SINGGLE 21
#define PLAYMODE_LIST 24
#define VOL_UP 903
#define VOL_DOWN 902
#define ProgressBar_0 10000
#define ProgressBar_100 10100

	//printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));

	Int_controlCode= get_int_by_path(json_root,"/data/controlCode"); 
	if ( Int_controlCode < 0)
	{
		gatPrintf( GAT_DEBUG,"can't get controlCode.\n");	
		return -1;
	}
	gatPrintf( GAT_DEBUG,"controlCode: %d\n",Int_controlCode);
	Int_fromUser= get_int_by_path(json_root,"/data/fromUser"); 
	if ( Int_fromUser < 0)
	{
		gatPrintf( GAT_DEBUG,"can't get fromUsercontrolCode.\n");	
		return -1;
	}
	gatPrintf( GAT_DEBUG,"fromUser: %d\n",Int_fromUser);
	//Int_controlCode = atoi(controlCode);
	if (Int_controlCode >= 0 && Int_controlCode <= VOL )
	{
		//TODO send vol control msg.
		volueIndex = Int_controlCode;
		gatPrintf( GAT_DEBUG,"set VOL to : %d\n",volueIndex);
		memset(buf,0,128);
		sprintf(buf,"/etc/adckey/adckey_function.sh homeServerVolume %d", sys_volume_tab[volueIndex]);
		
		printf("%s:%d \n", __func__,__LINE__);
		system(buf);
		
// the code below will case segmentation fault
#if 0
		memset(volume,0,5);
		
		read_file("/etc/system_volume",(char *)volume);
		
		if (volume != NULL)
		{
			p_songNode->volume = atoi(volume);
		}
		ret = package_json_pushdevinfo_action();
		printf("%s:%d pushdevinfo return: %d\n", __func__,__LINE__,ret);
#endif
	}
	else if (Int_controlCode == START)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"START\n");
		type = SOURCE_TYPE_PUSHCONTROLE_START;
		evt = EVT_PLAYER_SWITCH;
		statusEventNotify(evt, &type);
	}
	else if (Int_controlCode == PAUSE)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"PAUSE\n");
		type = SOURCE_TYPE_PUSHCONTROLE_PAUSE;
		evt = EVT_PLAYER_SWITCH;
		statusEventNotify(evt, &type);
	}
	else if (Int_controlCode == VOL_UP)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"VOL+\n");
	//	system("/etc/adckey/adckey_function.sh volumeup");
		if(volueIndex<10)
			volueIndex++;
		memset(buf,0,128);
		sprintf(buf,"/etc/adckey/adckey_function.sh homeServerVolume %d",sys_volume_tab[volueIndex]);
		system(buf);

	}
	else if (Int_controlCode == VOL_DOWN)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"VOL-\n");
//		system("/etc/adckey/adckey_function.sh volumedown");
		if(volueIndex > 0)
			volueIndex--;
		memset(buf,0,128);
		sprintf(buf,"/etc/adckey/adckey_function.sh homeServerVolume %d",sys_volume_tab[volueIndex]);
		system(buf);
	}
	else if (Int_controlCode == PRE)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"PRE\n");
		type = SOURCE_TYPE_PUSHCONTROLE_PRE;
		evt = EVT_PLAYER_SWITCH;
		statusEventNotify(evt, &type);
	}
	else if (Int_controlCode == NEXT)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"NEXT\n");
		type = SOURCE_TYPE_PUSHCONTROLE_NEXT;
		evt = EVT_PLAYER_SWITCH;
		statusEventNotify(evt, &type);
	}

	else if (Int_controlCode >= PLAYMODE_SINGGLE &&  Int_controlCode <= PLAYMODE_LIST)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"set Play Mode to %d\n",Int_controlCode);
		g_play_list_info.playMode= Int_controlCode;
	}

	else if (Int_controlCode >= ProgressBar_0 &&  Int_controlCode <= ProgressBar_100)
	{
		//TODO send vol control msg.
		gatPrintf( GAT_DEBUG,"set progress to %d%\n",(Int_controlCode-ProgressBar_0));
		//seek_absolute_pos((Int_controlCode-ProgressBar_0));
	}
	json_object_put(json_root);
	//TODO send action to player thread.
	return 0;
}

int syncImsInfo_hander(json_object * json_root)
{
	printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));
	return 0;
}
int syncImsBindStatus_hander(json_object * json_root)
{
	int status ;
	int count = 0;

	printf("%s: get data:%s\n",__func__,json_object_to_json_string_ext(json_root,2));

	status = get_int_by_path(json_root, "/data/status");
	if (status < 0)
	{
		printf("get phone status error.\n");
		return 0;
	}
		
	//while(pthread_mutex_trylock(&p_cloudphone->mutex))
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
	printf("get phone lock OK.\n");
	if (status == 1)
	{
		printf("start cloudphone regist.\n");
	//	p_cloudphone->cmd = PHONE_INIT;
		cloudphone_cmd = PHONE_INIT;
		gatTimerAdd(getGetPhoneListTimer(), "getPhoneListTimer",3,getPhoneListTimerCb,(void*)(getMiguClientNode()->fd));
	}
	else
	{
		printf("unregist cloudphone.\n");
		//p_cloudphone->cmd = PHONE_DEREGIST;
		cloudphone_cmd = PHONE_DEREGIST;
	}
	//pthread_cond_signal(&p_cloudphone->condition);
	//pthread_mutex_unlock(&p_cloudphone->mutex);

	pthread_cond_signal(&cloudphone_condition);
	pthread_mutex_unlock(&cloudphone_mutex);

	return 0;
}
int syncImsPhoneList_hander(json_object * json_root)
{
	int ret=-1;
	ret=updateImsPhoneList(json_root);
	return ret;
}
int pushMdevUser_hander(json_object * json_root)
{
	int32 ret = 0;
	char * memberPhone=NULL;
	memberPhone = get_string_by_path(json_root,"/data/memberPhone");
	ret = update_memberPhone_to_file(memberPhone);

	ret = home_server_init();

	return 0;
}

int getBookInfo_byId(json_object * json_root)
{
	bookNode_t* p_bookNode = &bookData;
	aiMessage_t* p_aiMessage = &aiMessage;
	struct json_object* ai_result=NULL;
	int isFree;
	char * payUrl=NULL;
	char * chapterUrl;
	char * nextId; 
	char * prevId; 
	char * contentName;
	char * chapterName;
	char * contentId;
	char * chapterId;
	char * readerName; 
	char * contentPicUrl; 
	int32 type;

	//printf("%s:%d get book string:%s\n",__func__,__LINE__,
	//		json_object_to_json_string_ext(json_root,2));

	gatTimerDel(getRequestHandleTimer());
	isFree = get_int_by_path(json_root, "/data/isFree");
//	if (isFree == 1)
//	{
	payUrl = get_string_by_path(json_root, "/data/payUrl");
	if (payUrl == NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot tget chapterUrl failed\n");

		type = TYPE_RESULT;
		sendInterruptEvt(type);
		return -1;
	}
	//}
	chapterUrl = get_string_by_path(json_root, "/data/chapterUrl");
	if (chapterUrl == NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot tget chapterUrl failed\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}
	nextId = get_string_by_path(json_root, "/data/nextId");
	prevId = get_string_by_path(json_root, "/data/prevId");
	if (nextId == NULL || prevId == NULL ) {
		gatPrintf( GAT_DEBUG,"Error: ting boot get nextId prev failed\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}

	contentName = get_string_by_path(json_root, "/data/contentName");
	if (contentName == NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot get contentName failed\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}

	chapterName = get_string_by_path(json_root, "/data/chapterName");
	if (chapterName == NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot get chapterName failed\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}

	chapterId = get_string_by_path(json_root, "/data/chapterId");
	if (chapterId== NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot get failed\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}
	contentId = get_string_by_path(json_root, "/data/contentId");
	if ( contentId == NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot get contentId failed\n");
		type = TYPE_RESULT;
		sendInterruptEvt(type);

		return -1;
	}

	contentPicUrl = get_string_by_path(json_root, "/data/contentPicUrl");
	if ( contentPicUrl == NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot get contentId failed\n");
#if 0
		evt = EVT_CAE_WAKEUP;
		type = TYPE_SEMANTIC_NOTFOUND;
		len = 0;
		notifyWakeupEvt(evt, type, len);
		return -1;
#endif
	}

	readerName = get_string_by_path(json_root, "/data/readerName");
	if ( readerName == NULL) {
		gatPrintf( GAT_DEBUG,"Error: ting boot get readerName failed\n");
#if 0
		evt = EVT_CAE_WAKEUP;
		type = TYPE_SEMANTIC_NOTFOUND;
		len = 0;
		notifyWakeupEvt(evt, type, len);
		return -1;
#endif
	}

	iofLock(&p_bookNode->bookLock);
	p_bookNode->chapterUrl=chapterUrl;
	p_bookNode->payUrl=payUrl;
	//gatPrintf( GAT_DEBUG,"getBookInfo_byId nextId:%s,prevId%s\n",p_bookNode->nextId,p_bookNode->prevId);
	memset(p_bookNode->nextId,0,20);
	memset(p_bookNode->prevId ,0,20);
	if (nextId != NULL &&  strlen(nextId) > 1)
	{
		memcpy(p_bookNode->nextId,nextId,strlen(nextId));
	}
	if (prevId != NULL &&  strlen(prevId) > 1)
	{
		memcpy(p_bookNode->prevId ,prevId ,strlen(prevId));
	}
	if (contentName != NULL &&  strlen(contentName ) > 1)
	{
		memset(p_bookNode->contentName ,0,128);
		memcpy(p_bookNode->contentName ,contentName,strlen(contentName ));
	}
	if (chapterName != NULL &&  strlen(chapterName ) > 1)
	{
		memset(p_bookNode->chapterName ,0,128);
		memcpy(p_bookNode->chapterName ,chapterName ,strlen(chapterName));
	}

	iofUnlock(&p_bookNode->bookLock);

	if(strlen(p_bookNode->payUrl) > 6 || 1==is_end_with(p_bookNode->chapterUrl,"wt=txt"))
	{
		statusEventNotify(EVT_PLAY_TEXTBOOK, (bookNode_t*)&p_bookNode);
		return 0;
	}
	statusEventNotify(EVT_BOOKPLAYER_ACTION, (bookNode_t*)&p_bookNode);
	ai_result = json_object_new_object();
	if(ai_result ==NULL)
	{
		return -1;
	}
	json_object_object_add(ai_result, "contentName", json_object_new_string(p_bookNode->contentName));
	json_object_object_add(ai_result, "chapterName", json_object_new_string(p_bookNode->chapterName));
	json_object_object_add(ai_result, "chapterId", json_object_new_string(chapterId));
	json_object_object_add(ai_result, "contentId", json_object_new_string(contentId));
	json_object_object_add(ai_result, "readerName", json_object_new_string(readerName));
	json_object_object_add(ai_result, "contentPicUrl", json_object_new_string(contentPicUrl));
	json_object_object_add(ai_result, "payUrl", json_object_new_string(""));
	json_object_object_add(ai_result, "isFree", json_object_new_int(isFree));

	p_aiMessage->result = ai_result;
#if 0
	gatPrintf( GAT_DEBUG,"chapterUrl:%s\n",p_bookNode->chapterUrl);
	gatPrintf( GAT_DEBUG,"nextId:%s\n",p_bookNode->nextId);
	gatPrintf( GAT_DEBUG,"prevId:%s\n",p_bookNode->prevId);
	gatPrintf( GAT_DEBUG,"contentName:%s\n",p_bookNode->contentName);
	gatPrintf( GAT_DEBUG,"chapterName :%s\n",p_bookNode->chapterName );
	gatPrintf( GAT_DEBUG,"isFree:%d\n",p_bookNode->isFree);
	gatPrintf( GAT_DEBUG,"payUrl:%s\n",p_bookNode->payUrl);
#endif

	return 0;
}

int getDevInfo_hander(json_object * json_root)
{
	int ret;
	//printf("%s:%d get music string:%s\n",__func__,__LINE__,
	//		json_object_to_json_string_ext(json_root,2));

	if( getDevStatus() & DEV_STATUS_PLAYMUSIC)
	{
		ret = package_json_pushdevinfo_action();
		printf("%s:%d pushdevinfo return: %d\n", __func__,__LINE__,ret);
	}
	return 0;
}
int play_book_next(int is_prev)
{
	char * nextId= NULL;
	struct BookRequestInfo book_request_info;
	bookNode_t* p_bookNode = &bookData;
	if(is_prev == 1)
	{
		nextId = p_bookNode->prevId;
		if(strlen(nextId) < 2)
		{
			if(getPlayType()==PLAYTYPE_BOOK)
			{
				printf("%s,%d\n", __func__, __LINE__);
				playermanager_play_text("抱歉，已经是第一章");
			}
			else
			{
				printf("%s,%d\n", __func__, __LINE__);
				playermanager_play_text("抱歉，已经是第一章");
			}
			return -1;
		}
	}
	else
	{
		nextId = p_bookNode->nextId;
		if(strlen(nextId) < 2)
		{
			if(getPlayType()==PLAYTYPE_BOOK)
			{
				playermanager_play_text("抱歉，已经是最后一章");
			}
			else
			{
				playermanager_play_text("抱歉，已经是最后一章");
			}
			return -1;
		}
	}

	book_request_info.chapterId = nextId; 
	request_book_info_by_Id(&book_request_info);

	return 0;
}
/* 
param1: recall phone number
param2: callback phone number
0: call by number
1: call by user name
*/
int update_recallCallbackNo_to_file(char * recallNo, char* callbackNo, int type)
{
	json_object  * data_root;
	int ret=-1;

	if(recallNo==NULL&&callbackNo==NULL)
	{
		gatPrintf( GAT_DEBUG,"phone no invalid!\n");
		return -1;
	}

	if(type!=0&&type!=1)
	{
		gatPrintf( GAT_DEBUG,"call type no invalid!\n");
		return -1;
	}

	//have old data.	
	if (access(TEL_FILE, F_OK) == 0)
	{
		data_root = json_object_from_file(TEL_FILE);
		if(data_root != NULL)
		{
			if(recallNo!=NULL)
			{
				json_object_object_del(data_root, "recallNumber");
				json_object_object_add(data_root,"recallNumber",json_object_new_string(recallNo));
				if(type == 0)
				{
					json_object_object_del(data_root, "recallType");
					json_object_object_add(data_root,"recallType",json_object_new_int(0));
				}
				else if(type == 1)
				{
					json_object_object_del(data_root, "recallType");
					json_object_object_add(data_root,"recallType",json_object_new_int(1));
				}
			}
			else
			{
				json_object_object_del(data_root, "callbackNumber");
				json_object_object_add(data_root,"callbackNumber",json_object_new_string(callbackNo));
				if(type == 0)
				{
					json_object_object_del(data_root, "callbackType");
					json_object_object_add(data_root,"callbackType",json_object_new_int(0));
				}
				else if(type == 1)
				{
					json_object_object_del(data_root, "callbackType");
					json_object_object_add(data_root,"callbackType",json_object_new_int(1));
				}
			}
		}
		else
		{
			remove(TEL_FILE);
			data_root = json_object_new_object(); 
			if(recallNo!=NULL)
			{
				json_object_object_add(data_root,"recallNumber",json_object_new_string(recallNo));
				if(type == 0)
				{
					json_object_object_del(data_root, "recallType");
					json_object_object_add(data_root,"recallType",json_object_new_int(0));
				}
				else if(type == 1)
				{
					json_object_object_del(data_root, "recallType");
					json_object_object_add(data_root,"recallType",json_object_new_int(1));
				}
			}
			else
			{
				json_object_object_add(data_root,"callbackNumber",json_object_new_string(callbackNo));
				if(type == 0)
				{
					json_object_object_del(data_root, "callbackType");
					json_object_object_add(data_root,"callbackType",json_object_new_int(0));
				}
				else if(type == 1)
				{
					json_object_object_del(data_root, "callbackType");
					json_object_object_add(data_root,"callbackType",json_object_new_int(1));
				}
			}
		}
	}
	else
	{
		data_root = json_object_new_object(); 
		if(recallNo!=NULL)
		{
			json_object_object_add(data_root,"recallNumber",json_object_new_string(recallNo));			
			if(type == 0)
			{
				json_object_object_del(data_root, "recallType");
				json_object_object_add(data_root,"recallType",json_object_new_int(0));
			}
			else if(type == 1)
			{
				json_object_object_del(data_root, "recallType");
				json_object_object_add(data_root,"recallType",json_object_new_int(1));
			}
		}
		else
		{
			json_object_object_add(data_root,"callbackNumber",json_object_new_string(callbackNo));			
			if(type == 0)
			{
				json_object_object_del(data_root, "callbackType");
				json_object_object_add(data_root,"callbackType",json_object_new_int(0));
			}
			else if(type == 1)
			{
				json_object_object_del(data_root, "callbackType");
				json_object_object_add(data_root,"callbackType",json_object_new_int(1));
			}
		}
	}
	ret = json_object_to_file_ext(TEL_FILE,data_root,2);
	if(ret < 0)
	{
		gatPrintf( GAT_DEBUG,"json_data write to file error.\n");
		return -1;
	}
	return ret;
}

/* 
return: call type
*/
int get_recallCallback_number(char* action, char* getNumberName)
{
	json_object *data_root;
	int callType=-1;
	char * callNumber=NULL;

	if (access(TEL_FILE, F_OK) == 0)
	{
		data_root = json_object_from_file(TEL_FILE);
		printf("%s,%d!\n",__FUNCTION__,__LINE__);
		if(data_root != NULL)
		{
			if(!strcmp(action, "REDIAL"))//RECALL,for recall type,just test,
			{
				callNumber = json_get_string_by_key(data_root,"recallNumber"); 				
				printf("%s,%d!recallNumber:%s\n",__FUNCTION__,__LINE__, callNumber);
				if(callNumber != NULL && strlen(callNumber)>0)
				{
					callType = json_get_int_by_key(data_root,"recallType"); 
					memcpy(getNumberName,callNumber,strlen(callNumber));
					json_object_put(data_root);
					return callType;
				}
			}
			else if(!strcmp(action, "CALLBACK"))//for callback
			{
				callNumber = json_get_string_by_key(data_root,"callbackNumber"); 
				printf("%s,%d!callNumber:%s\n",__FUNCTION__,__LINE__, callNumber);
				if(callNumber != NULL && strlen(callNumber)>0)
				{
					callType = json_get_int_by_key(data_root,"callbackType"); 
					memcpy(getNumberName,callNumber,strlen(callNumber));
					json_object_put(data_root);
					return callType;
				}
			}
			else
			{
				printf("%s,%d!\n",__FUNCTION__,__LINE__);
				return -1;
			}
		}
		else
		{
			printf("%s,%d!\n",__FUNCTION__,__LINE__);
			return -1;
		}
	}
	return -1;
}

int json_parser(const char * json_string, int dataLen)
{
	struct json_object * json_root;
	char * msgType=NULL;
	int int_msgType;
	char* recode = NULL;
	char* msg = NULL;
	
	if (json_string == NULL)
		return -1;
	json_root = json_tokener_parse(json_string); 
	if (json_root == NULL)
	{
		gatPrintf( GAT_DEBUG,"json data parse error.\n");
		return -1;
	}
	
	printf("###################### recv json #########################\n");
	printf("%s\n", json_object_to_json_string_ext(json_root,2));
	printf("############################################################\n");

	//首先判断设备ID是否存在
	recode = get_string_by_path(json_root,"/recode");
	if (recode != NULL) {
		msg = get_string_by_path(json_root,"/msg");
		gatPrintf( GAT_DEBUG,"recode:%s, msg:%s\n", recode, msg);
		if (!strcmp(recode, "6")) {
			if (msg == NULL) 
				playermanager_play_text("MSG信息缺失");
			else 
				playermanager_play_text(msg);
			return 0;
		}
	}

	//1. get msgType
	msgType = json_get_string_by_key(json_root,"msgType");	
	if(msgType == NULL )
	{
		jsonPrintf( GAT_ERROR,"%s:%d msgType error!\n",__func__,__LINE__);
		return -1;
	}

	//2. msgType to enum
	int_msgType = msgType_to_int(msgType);
	//
	//3. switch enum
	switch (int_msgType)
	{
		case LOGIN:
			login_hander(json_root);
			break;
		case HEART_BEAT:
			heartBeat_hander(json_root);
			break;
		case PUSH_DEV_INFO:
			pushDevInfo_hander(json_root);
			break;
		case SEMANTIC:
			semantic_hander(json_root);
			break;
		case GET_IMS_INFO:
			getImsInfo_hander(json_root);
			break;
		case GET_IMS_PHONE_LIST:
			getImsPhoneList_hander(json_root);
			break;
		case PUSH_AI_MESSAGE:
			pushAiMessage_hander(json_root);
			break;
		case GET_MDEV_USER:
			getMdevUser_hander(json_root);
			break;
		case PUSH_MUSICS:
			pushMusics_hander(json_root);
			break;
		case PUSH_CONTROL:
			pushControl_hander(json_root);
			break;
		case SYNC_IMS_INFO:
			syncImsInfo_hander(json_root);
			break;
		case SYNC_IMS_BIND_STATUS:
			syncImsBindStatus_hander(json_root);
			break;
		case SYNC_IMS_PHONE_LIST:
			syncImsPhoneList_hander(json_root);
			break;
		case PUSH_MDEV_USER:
			pushMdevUser_hander(json_root);
			break;
		case GET_MUSICINFO_BYID:
			getBookInfo_byId(json_root);
			break;
		case GET_DEV_INFO:
			getDevInfo_hander(json_root);
			break;
	default:
			break;
	}
	//json_object_put(json_root);
	return 0;
}

