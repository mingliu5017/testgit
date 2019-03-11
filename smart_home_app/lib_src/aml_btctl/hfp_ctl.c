/****************************************************************************
 **
 **  Name:           hfp_ctl.c
 **
 **  Description:    Bluetooth Manager application
 **
 **
 **
 **
 *****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "hfp_ctl_socket.h"
#include "aml_log.h"
#include "hfp_ctl.h"

#define LOG_TAG "HFP"

//#define INFO(fmt, args...) \
//	printf("[HFP_CTL][%s] " fmt, __func__, ##args)


static tAPP_SOCKET hfp_ctl_sk;
static unsigned int sVol = 10;
static unsigned int mVol = 10;
static int hfp_connected = 0;
static pthread_t phfp_ctl_id;
static void *hfp_ctl_monitor(void *arg);
static hfp_event_cb event_cb = NULL;

#define CTL_SOCKET_PATH "/etc/bluetooth/hfp_ctl_sk"


void hfp_set_event_cb(hfp_event_cb callback)
{
	event_cb = callback;
}


int hfp_is_connected(void)
{
	return hfp_connected;
}


int hfp_ctl_init(void)
{
	strcpy(hfp_ctl_sk.sock_path, CTL_SOCKET_PATH);
	if (setup_socket_server(&hfp_ctl_sk)) {
		INFO("setup socket server fail\n");
		return -1;
	}

	//wait for socket ready
	usleep(1000);
	if (pthread_create(&phfp_ctl_id, NULL, hfp_ctl_monitor, NULL)) {
		perror("hfp_ctl_server");
		teardown_socket_server(&hfp_ctl_sk);
		return -1;
	}

	INFO("Server is ready for connection...\n");
	return 0;
}

void hfp_ctl_delinit(void)
{
	/*set thread to cancel state, and free resouce
	  thread will exit once teardown_socket_server()
	  wakes thread up from recv()
	 */
	pthread_cancel(phfp_ctl_id);
	pthread_detach(phfp_ctl_id);

	hfp_connected = 0;
	teardown_socket_server(&hfp_ctl_sk);
}

int hfp_answer_call(void)
{
	unsigned int byte, ret = 1;
	char msg[] = "A";
	INFO("\n");
	byte = socket_send(hfp_ctl_sk.client_sockfd, msg, strlen(msg));

	if (byte == strlen(msg))
		ret = 0;

	return ret;
}

int hfp_reject_call(void)
{
	unsigned int byte, ret = 1;
	char msg[] = "+CHUP";
	INFO("\n");
	byte = socket_send(hfp_ctl_sk.client_sockfd, msg, strlen(msg));
	if (byte == strlen(msg))
		ret = 0;

	return ret;
}

int hfp_VGS_up(void)
{
	sVol += 1;
	return hfp_set_VGS(sVol);
}

int hfp_VGS_down(void)
{
	sVol -= 1;
	return hfp_set_VGS(sVol);

}

int hfp_VGM_up(void)
{
	mVol += 1;
	return hfp_set_VGM(mVol);
}

int hfp_VGM_down(void)
{
	mVol -= 1;
	return hfp_set_VGM(mVol);

}

int hfp_set_VGS(int value)
{
	unsigned int byte, ret = 1;
	char msg[10] = {0};

	//value range from 0 ~ 15
	value = value < 0 ? 0 : value;
	value = value > 15 ? 15 : value;

	sprintf(msg, "+VGS=%d", value);
	INFO("\n");
	byte = socket_send(hfp_ctl_sk.client_sockfd, msg, strlen(msg));
	if (byte == strlen(msg))
		ret = 0;

	return ret;
}

int hfp_set_VGM(int value)
{
	unsigned int byte, ret = 1;
	char msg[10] = {0};

	//value range from 0 ~ 15
	value = value < 0 ? 0 : value;
	value = value > 15 ? 15 : value;

	sprintf(msg, "+VGM=%d", value);
	INFO("\n");
	byte = socket_send(hfp_ctl_sk.client_sockfd, msg, strlen(msg));
	if (byte == strlen(msg))
		ret = 0;

	return ret;
}

static void *hfp_ctl_monitor(void *arg)
{
	int byte, value = -1, event = -1, i;
	char msg[64];


accept:

	if (accpet_client(&hfp_ctl_sk) < 0) {
		INFO("accept client fail\n");
		return NULL;
	}

	INFO("recieving....\n");
	while(1) {
		memset(msg, 0, sizeof(msg));
		byte = recv(hfp_ctl_sk.client_sockfd, msg, sizeof(msg), 0);
		if (byte < 0) {
			perror("recv");
			continue;
		}
		if (byte == 0) {
			INFO("client off line\n");
			goto accept;
		}

		if (byte == 2) {
			event   = msg[0];
			value = msg[1];
			INFO("event = %d, value = %d\n", event, value);
		} else {
			INFO("invalid msg: %s\n", msg);
			for (i = 0 ; i < byte; i++)
				INFO("msg %d = %d\n", i, msg[i]);
			continue;
		}

		switch (event) {
			case HFP_EVENT_CONNECTION:
				switch (value) {
					case HFP_IND_DEVICE_DISCONNECTED:
						INFO("HFP disconnected!!!\n");
						hfp_connected = 0;
						break;
					case HFP_IND_DEVICE_CONNECTED:
						INFO("HFP connected!!!\n");
						hfp_connected = 1;
						break;
				}
				break;

			case HFP_EVENT_CALL:
				switch (value) {
					case HFP_IND_CALL_NONE:
						INFO("Call stopped!!!\n");
						break;
					case HFP_IND_CALL_ACTIVE :
						INFO("Call active!!!\n");
						break;
				}
				break;

			case HFP_EVENT_CALLSETUP:
				switch (value) {
					case HFP_IND_CALLSETUP_NONE:
						INFO("Callsetup stopped!!!\n");
						break;
					case HFP_IND_CALLSETUP_IN :
						INFO("An incomming Callsetup!!!\n");
						break;
					case HFP_IND_CALLSETUP_OUT :
						INFO("An outgoing Callsetup!!!\n");
						break;
				}
				break;
			case HFP_EVENT_VGS:
				INFO("VGS EVENT!!!\n");
				sVol = value;
				break;
			case HFP_EVENT_VGM:
				INFO("VGM EVENT!!!\n");
				mVol = value;
				break;
		}
		if(event_cb)
			event_cb(event, value);
	}
	INFO("exit\n");
	return NULL;
}
/*
//test function
void my_hfp_event_cb(int event, int value)
{
	LOG(LEVEL_INFO,"event:%d  value:%d",event,value);
}

int hfp_test(void)
{
	hfp_set_event_cb(my_hfp_event_cb);
	if (hfp_ctl_init())
		return -1;

	sleep(1);

	while(!hfp_is_connected())
		sleep(1);

	hfp_answer_call();
	sleep(2);
	hfp_reject_call();

	hfp_VGS_up();
	hfp_VGM_up();
	hfp_VGS_down();
	hfp_VGM_up();
	sleep(10);
	hfp_ctl_delinit();

}
*/


