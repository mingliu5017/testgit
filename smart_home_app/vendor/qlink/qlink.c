#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <json_engine.h>
#include <json-c/json.h>
#include <unistd.h>

#include "common.h"
#include "Qlink_API.h"
#include "homeserver.h"
#include "statusevent_manage.h"
#include "network_cfg.h"

#define COAPSERIP       "coap://192.168.1.1"
#define PORTSTR         "5683"

#define COAPSEARCHGW    "/qlink/searchgw"
#define COAPREGISTER    "/device/inform/bootstrap"
#define COAPBOOT		"/device/inform/boot"
#define QLINK_FILE		"/data/qlink_info.json"
#define HTTPS_REG		"/device/inform/bootstrap"
#define HTTPS_BOOT		"/device/inform/boot"
#define NOTIFY_OK		"/qlink/regist"

static char *deviceId = NULL;
char * deviceType = "31123";
char * productToken = "VNPlqO7sXImPczvy";
char * firmwareVersion = "Linux-4.9.68";
char  g_user_key[65] = {0};
char  g_gwAddress2[65] ={0};
char *g_config_data = NULL;
static int got_passwd=0;

void *Coap_server_Thread(void *argc)
{    // 启动qlinkserver
	printf("%s\n",__func__);
    Qlink_StartCoapServer();
	printf("%s end\n",__func__);
    return ((void *)0);
}

int  CGW_register()
{
	char url[256] = { 0 };
	char data[256] = { 0 };
	time_t timestamp;
	printf("%s\n",__func__);

	//sprintf(url,"%s%s", gwAddress2, HTTPS_REG );
	sprintf(url,"%s%s", g_gwAddress2, HTTPS_REG );

	deviceId = get_BT_Mac_addr();
	
	timestamp = time(NULL);
	sprintf(data,
		"{\"deviceMac\":\"%s\",\"deviceType\":\"%s\",\"productToken\":\"%s\",\"timestamp\":%ld}",
		deviceId, deviceType, productToken, timestamp);

	printf("%s:url:%s \ndata:%s\n", __func__, url, data);
	httppostcallback(url, g_user_key, data);

	return 0;
}

int  CGW_boot()
{
	char url[256] = { 0 };
	char data[256] = { 0 };
    char ipAddress[20];
	time_t timestamp;
	printf("%s\n",__func__);

	snprintf(url, 256, "%s%s", g_gwAddress2, HTTPS_BOOT );

	deviceId = get_BT_Mac_addr();
	
	memset(ipAddress,0,20);
	get_ip_addr(ipAddress);

	timestamp = time(NULL);

	snprintf(data, 256,
		 "{\"deviceId\":\"%s\",\"deviceType\":\"%s\",\"firmwareVersion\":\"%s\",\"softwareVersion\":\"%s\",\"ipAddress\":\"%s\",\"timestamp\":%ld}",
		 deviceId, deviceType, firmwareVersion, VERSION, ipAddress,
		 timestamp);

	printf("%s:url:%s \ndata:%s\n",__func__,url,data);
	httppostcallback( url, NULL, data);

	return 0;
}

int write_config_to_file(const char *SSID, const char *passwd)
{
	const char *wifi_config_file = "/data/select.txt";
	FILE *fp;		/*定义一个文件指针 */
	int i;

	printf("%s\n", __func__);
	fp = fopen(wifi_config_file, "w+");
	if (fp == NULL)		/*判断文件是否打开成功 */
		puts("File open error");	/*提示打开不成功 */

	fprintf(fp, "%s\n%s\n", SSID, passwd);

	fflush(fp);

	i = fclose(fp);		/*关闭打开的文件 */
	if (i == 0)		/*判断文件是否关闭成功 */
		puts("file close OK");	/*提示关闭成功 */
	else
		puts("File close error");	/*提示关闭不成功 */
}

int set_wifi_config(const char *SSID, const char *passwd)
{
	const char *f2fs_path = "/usr/bin/wifi_setup_bt.sh";
	char cmd[50] = { 0 };

	printf("%s\n", __func__);
	// write config to file.
	write_config_to_file(SSID, passwd);

	//set config from file.
	sprintf(cmd, "%s", f2fs_path);
	int status = system(cmd);
	printf("system return: %d\n", status);
	return 0;

}
int JoinHideSSID()
{
	int count = 0;
	//第三方实现: 加入隐藏SSID：CMCC-QLINK  pwd：sD5fH%9k
	printf("%s\n", __func__);
	set_wifi_config("CMCC-QLINK", "NONE");

	while (0 == check_wifi_status()) {
		printf("%s,set wifi_config 1 start.\n", __func__);
		set_wifi_config("CMCC-QLINK", "NONE");
		printf("%s,set wifi_config 1 end.\n", __func__);
		sleep(2);
		if (count++ > 29) {
			printf("%s:%d: connect to CMCC-QLINK failed.\n",
			       __func__, __LINE__);
			return -1;
		}
	}
	return 0;
}

// 收到智能网关的回复
int BootCallback(unsigned char *data, int len)
{
	printf("%s: data:%s,len:%d\n", __func__, data, len);
	printf("%s\n", __func__);
	//todo 根据回复结果进行处理
	return 1;
}

//return 1 if wifi connect
//return 0 if wifi disconnect
#if 0
int check_wifi_status()
{
	char ipAddress[20] = { 0 };
	if (NULL == get_ip_addr(ipAddress))
		return 0;
	else
		return 1;
}
#endif
#if 0
char *Qlink_sendRequest(char *url, char *data, char **response, int *len)
{
	char *reponse = NULL;
	int replen;
	if (Qlink_SendCaopRequest(url, "post", data, &reponse, &replen) ==
	    -1) {
		if (reponse)
			free(reponse);
		return NULL;
	} else {
		if (reponse) {
			return reponse;
		} else
			return NULL;
	}
}
}
#endif
//6.2.1
//int Qlink_Register(const char* deviceId, const char* deviceType, const char* productToken, const char* timestamp)
int Qlink_Register()
{
	char url[500] = { 0 };
	char *reponse = NULL;
	int replen;
	time_t timestamp;
	int ret = 0;

	//snprintf(url, 500, "%s:%s%s", COAPSERIP, PORTSTR, COAPREGISTER);
	snprintf(url, 500, "%s%s", COAPSERIP, COAPREGISTER);

	char data[500] = { 0 };
	json_object *jo1;
	deviceId = get_BT_Mac_addr();

	timestamp = time(NULL);
	snprintf(data, 500,
		 "{\"deviceMac\":\"%s\",\"deviceType\":\"%s\",\"productToken\":\"%s\",\"timestamp\":\"%ld\"}",
		 deviceId, deviceType, productToken, timestamp);

	printf("%s: send data:\n%s\n", __func__, data);
	if (Qlink_SendCaopRequest(url, "post", data, &reponse, &replen) ==
	    -1) {
		if (reponse)
			free(reponse);
		return -1;
	} else {
		if (reponse == NULL) {
			printf("%s: rev_data is NULL\n", __func__);
			return -1;
		}

		printf("%s: rev_data:%s,len:%d\n", __func__, reponse,
		       replen);
		jo1 = json_tokener_parse((const char *) reponse);
		ret = json_object_to_file_ext(QLINK_FILE, jo1, 2);
	}
	return 0;
}

int Qlink_Boot()
{
	char url[500] = { 0 };
	char ipAddress[20];
	char *reponse = NULL;
	int replen;
	time_t timestamp;
	json_object *jo1;
	snprintf(url, 500, "%s%s", COAPSERIP, COAPBOOT);

	char data[500] = { 0 };

	deviceId = get_BT_Mac_addr();

	memset(ipAddress, 0, 20);
	get_ip_addr(ipAddress);

	timestamp = time(NULL);

	snprintf(data, 500,
		 "{\"deviceId\":\"%s\",\"deviceType\":\"%s\",\"firmwareVersion\":\"%s\",\"softwareVersion\":\"%s\",\"ipAddress\":\"%s\",\"timestamp\":\"%ld\"}",
		 deviceId, deviceType, firmwareVersion, VERSION, ipAddress,
		 timestamp);

	printf("%s: send data:\n%s\n", __func__, data);

	if (Qlink_SendCaopRequest(url, "post", data, &reponse, &replen) ==
	    -1) {
		if (reponse)
			free(reponse);
		return -1;
	} else {
		if (reponse == NULL) {
			printf("%s: rev_data is NULL\n", __func__);
			return -1;
		}
		printf("%s: rev_data:%s,len:%d\n", __func__, reponse,
		       replen);
		jo1 = json_tokener_parse((const char *) reponse);
	}
	return 0;
}


// 收到智能网关的请求
void *qlink_Thread(void *data)
{
	json_object *jo1;
	int count = 0;
	printf("%s: data:%s,\n",__func__,data);
    //数据格式为：
    // {"SSID":"CMCC-H901","password":"12345678","encrypt":"WPA"}
    // 收到后，根据ssid以及密码进行配置设备的网络:第三方实现
	jo1 = json_tokener_parse((const char *)data); 
    char *ssid;
    char *key;
    char *Enc;

	printf("%s\n",__func__);
	ssid = json_get_string_by_key(jo1,"SSID");
	key= json_get_string_by_key(jo1,"password");
	Enc = json_get_string_by_key(jo1,"encrypt");

	if (got_passwd == 0) {
		got_passwd = 1;
		int network_change_event = WIFICONFIG_GOT_PASSWORD;
		statusEventNotify(EVT_NETWORK_CHANGE,
				  &network_change_event);
		set_wifi_config(ssid, key);
		while (0 == check_wifi_status()) {
			printf("%s,set wifi_config start.\n", __func__);
			set_wifi_config(ssid, key);
			printf("%s,set wifi_config end.\n", __func__);
			sleep(3);
			if (count++ > 10) {
				printf
				    ("%s:%d: connect to %s,pw:%s failed.\n",
				     __func__, __LINE__, ssid, key);
				int network_change_event =
				    WIFICONFIG_CONNECT_FAIL;
				statusEventNotify(EVT_NETWORK_CHANGE,
						  &network_change_event);

				free(g_config_data);
				return -1;
			}
		}
		//配网成功/失败后,通知网关
		if (count < 10) {
			int network_change_event =
			    WIFICONFIG_CONNECT_SUCCESS;

			char bcast_addr[20] = { 0 };
			statusEventNotify(EVT_NETWORK_CHANGE,
					  &network_change_event);

			//5.3.6
			get_bcast_addr(bcast_addr);
			deviceId = get_BT_Mac_addr();
			Qlink_ackQlinkNetinfo(bcast_addr, deviceId,
					      deviceType);
			//6.2.1
			Qlink_Register();
			//6.2.3
			Qlink_Boot();

			//7.2
			ap_config_notify_success();
		}
	}

	free(g_config_data);
	return 0;
}

int ReciveInternetChannelCallback(unsigned char *data, int len)
{
	int ret;
	pthread_t qlink_id= NULL;
	printf("%s: len:%d data:%s,\n",__func__,len,data);
	g_config_data = malloc(512);
	memset(g_config_data, '\0', 512);
	memcpy(g_config_data, data, len);
	// 收到后，根据ssid以及密码进行配置设备的网络:第三方实现
	ret = pthread_create(&qlink_id, NULL, qlink_Thread, g_config_data);
	if (ret != 0) {
		printf ("create thread error!\n");
		return -1;
	}
	pthread_detach(qlink_id);

	return 0;
}

int ap_config_notify_success()
{
	char url[500] = { 0 };
	char data[500] = {0};
	char *reponse = NULL;
	int replen;
	char bcast_addr[20] = { 0 };

	get_bcast_addr(bcast_addr);
	snprintf(url, 500, "coap://%s%s", bcast_addr, NOTIFY_OK);

	deviceId = get_BT_Mac_addr();
	snprintf(data, 500,
		 "{\"deviceId\":\"%s\",\"deviceType\":\"%s\",\"respCode\":\"1\"}",
		 deviceId, deviceType);

	printf("%s: \nurl:%s\ndata:%s\n", __func__, url, data);

	if (Qlink_SendCaopRequest(url, "post", data, &reponse, &replen) ==
	    -1) {
		if (reponse)
			free(reponse);
		return -1;
	} else {
		if (reponse == NULL) {
			printf("%s: broadcase successs response is NULL\n",
			       __func__);
			return -1;
		}
		printf("%s: rev_data:%s,len:%d\n", __func__, reponse,
		       replen);
	}

	return 0;
}

#if 0
//数据格式为：
{
      "SSID": "HZJ", "password": "******", "encrypt": "*******", "channel": "******", "CGW":{
      "user_key": "*******", "gwAddress": "*******", "gwAddress2":"*******"}
}
#endif
int qlink_setep1()
{
    char ipAddress[20];
    int i,ret;

	printf("%s\n",__func__);

	got_passwd=0;
	deviceId = get_BT_Mac_addr();
    //设备上线后，无首选SSID，尝试加入默认的引导SSID
    //加入隐藏SSID:第三方实现
    ret = JoinHideSSID();
	if (ret < 0) {
		printf("#####################\n");
		printf("JoinHideSSID failed\n");
		printf("#####################\n");
	} else {
		memset(ipAddress, 0, 20);
		get_ip_addr(ipAddress);
		if (ipAddress == NULL) {
			printf("get_ip_addr failed\n");
		} else {
			printf("get_ip_addr %s\n", ipAddress);
		}

		//加入成功后，通知网关已加入引导上网通道
		if (deviceId != NULL) {
			Qlink_notifyGatewayJoinBoot(deviceId, deviceType,
						    ipAddress);
		} else {
			printf("get BT mac error.\n");
		}
	}

	printf("%s end\n", __func__);
	//关闭CoapServer
	return 0;
}

// AP 配网线程
void *ap_config_Thread(char *data)
{
	json_object *jobj = NULL;
	json_object *jCGW = NULL;
	int count = 0;
	char *ssid;
	char *key;
	char *Enc;
	char *gwAddress;
	char *gwAddress2;
	char *user_key;
	int ret;
#if 0
	struct json_tokener *tok;
	enum json_tokener_error jerr;
	int stringlen = 0;
	stringlen = strlen(data);
	printf("%s: indata: len:%d\n %s\n", __func__, stringlen, data);
	do {
		jobj = json_tokener_parse_ex(tok, data, stringlen);
	} while ((jerr =
		  json_tokener_get_error(tok)) == json_tokener_continue);
	if (jerr != json_tokener_success) {
		printf("Error: %s\n", json_tokener_error_desc(jerr));
		// Handle errors, as appropriate for your application.

		int network_change_event = WIFICONFIG_CONNECT_FAIL;
		statusEventNotify(EVT_NETWORK_CHANGE,
				  &network_change_event);
		return -1;
	}
	if (tok->char_offset < stringlen)	// XXX shouldn't access internal fields
	{
		// Handle extra characters after parsed object as desired.
		// e.g. issue an error, parse another object from that point, etc...
		int network_change_event = WIFICONFIG_CONNECT_FAIL;
		statusEventNotify(EVT_NETWORK_CHANGE,
				  &network_change_event);
		return -1;
	}
#else
	jobj = json_tokener_parse((const char *) data);
	if (jobj == NULL) {
		printf("get config json error.\n ");
		//json_tokener_get_error(tok);
		int network_change_event = WIFICONFIG_CONNECT_FAIL;
		statusEventNotify(EVT_NETWORK_CHANGE,
				  &network_change_event);
		free(g_config_data);
		return -1;
	}
#endif

	ssid = json_get_string_by_key(jobj, "SSID");
	key = json_get_string_by_key(jobj, "password");
	Enc = json_get_string_by_key(jobj, "encrypt");

	printf("get ssid :%s\n ", ssid);
	printf("get key:%s\n ", key);

	json_pointer_get(jobj, "/CGW", &jCGW);
	if (jCGW != NULL) {
		gwAddress = get_string_by_path(jCGW, "/gwAddress");
		gwAddress2 = get_string_by_path(jCGW, "/gwAddress2");
		user_key = get_string_by_path(jCGW, "/user_key");
		if (gwAddress2 == NULL || user_key == NULL) {
			printf("get gwAddress2 | user_key error\n ");
			int network_change_event = WIFICONFIG_CONNECT_FAIL;
			statusEventNotify(EVT_NETWORK_CHANGE,
					  &network_change_event);
			free(g_config_data);
			return -1;
		}
		printf("get gwAddress:%s\n ", gwAddress);
		printf("get gwAddress2:%s\n ", gwAddress2);
		sprintf(g_gwAddress2, "%s", gwAddress2);
		sprintf(g_user_key, "%s", user_key);
	}
	//ap_config_notify_get_msg();
	if (ssid == NULL) {

		printf("%s:%d: connect to %s,pw:%s failed.\n", __func__,
		       __LINE__, ssid, key);
		int network_change_event = WIFICONFIG_CONNECT_FAIL;
		statusEventNotify(EVT_NETWORK_CHANGE,
				  &network_change_event);
		free(g_config_data);
		return -1;
	}
	printf("config wifi\n ");
	system("/usr/bin/ap_config.sh stop");
	sleep(1);
	system("/etc/init.d/S42wifi up");
	sleep(3);
	//SetRelayBySsid(ssid,key, Enc);    
	if (got_passwd == 0) {
		got_passwd = 1;
		int network_change_event = WIFICONFIG_GOT_PASSWORD;
		statusEventNotify(EVT_NETWORK_CHANGE,
				  &network_change_event);
		set_wifi_config(ssid, key);
		while (0 == check_wifi_status()) {
			printf("%s,set wifi_config start.\n", __func__);
			set_wifi_config(ssid, key);
			sleep(3);
			if (count++ > 10) {
				printf
				    ("%s:%d: connect to %s,pw:%s failed.\n",
				     __func__, __LINE__, ssid, key);
				int network_change_event =
				    WIFICONFIG_CONNECT_FAIL;
				statusEventNotify(EVT_NETWORK_CHANGE,
						  &network_change_event);
				free(g_config_data);
				return -1;
			}
		}
		//配网成功/失败后,通知网关
		if (count < 10) {
			int network_change_event =
			    WIFICONFIG_CONNECT_SUCCESS;

			char bcast_addr[20] = { 0 };
			get_bcast_addr(bcast_addr);

			printf("%s: wifi config successs.\n", __func__);
			//本地网关流程
			if (jCGW == NULL) {
				//5.3.6
				get_bcast_addr(bcast_addr);
				deviceId = get_BT_Mac_addr();
				Qlink_ackQlinkNetinfo(bcast_addr, deviceId,
						      deviceType);
				//6.2.1
				Qlink_Register();
				//6.2.3
				Qlink_Boot();
				//7.2
				ap_config_notify_success();
			}
			//云网关流程
			else {
				//5.3.6
				get_bcast_addr(bcast_addr);
				deviceId = get_BT_Mac_addr();
				Qlink_ackQlinkNetinfo(bcast_addr, deviceId,
						      deviceType);
				//6.2.1
				CGW_register();
				//6.2.3
				CGW_boot();
				//7.2
				ap_config_notify_success();
			}
			sleep(1);
			statusEventNotify(EVT_NETWORK_CHANGE,
					  &network_change_event);
			//Qlink_ackQlinkNetinfo(deviceId,deviceType);
		}

	}

	free(g_config_data);
	return 0;

}

// 收到智能网关的请求
int ReciveNetInfoCallback(unsigned char *data, int len)
{
	int ret;
	pthread_t ap_config_id = NULL;
	printf("%s: len:%d data:\n%s\n", __func__, len, data);
	g_config_data = malloc(512);
	memset(g_config_data, '\0', 512);
	memcpy(g_config_data, data, len);
	// 收到后，根据ssid以及密码进行配置设备的网络:第三方实现
	ret =
	    pthread_create(&ap_config_id, NULL, ap_config_Thread,
			   g_config_data);
	if (ret != 0) {
		printf("create thread error!\n");
		return -1;
	}
	pthread_join(&ap_config_id, NULL);
	//free(data);
	//pthread_detach(ap_config_id);

	return 0;
}

int start_qlink()
{
	int i, ret;

	pthread_t Coap_server_id = NULL;
	pthread_t qlink_id = NULL;
	printf("%s\n", __func__);

	Qlink_setReciveInternetChannelCallback
	    (ReciveInternetChannelCallback);

	ret =
	    pthread_create(&Coap_server_id, NULL, Coap_server_Thread,
			   NULL);
	if (ret != 0) {
		printf("create thread error!\n");
		return -1;
		//exit(1);
	}
	pthread_detach(Coap_server_id);

	qlink_setep1();
#if 0
	ret = pthread_create(&qlink_id, NULL, qlink_Thread, NULL);
	if (ret != 0) {
		printf("create thread error!\n");
		return -1;
		//exit(1);
	}
	pthread_detach(qlink_id);
#endif

	return 0;

}

int start_ap_config()
{
    int i,ret;

	pthread_t Coap_server_id = NULL;
	pthread_t qlink_id= NULL;
	printf("%s\n",__func__);

	got_passwd=0;
    Qlink_setReciveInternetChannelCallback(ReciveNetInfoCallback);
	
	ret =
	    pthread_create(&Coap_server_id, NULL, Coap_server_Thread,
			   NULL);
	if (ret != 0) {
		printf("create thread error!\n");
		return -1;
	}
	pthread_detach(Coap_server_id);

	return 0;
}
