/***************************************************************************
** CopyRight: Amlogic             
** Author   : ming.liu@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#define LOG_TAG "key_event_handler"

#include "common.h"
#include "iofstring.h"
#include "aml_log.h"
#include "iofipc.h"
#include "iofpthread.h"
#include "key_event_handler.h"
#include "key_event_process.h"
#include "Qlink_API.h"
#include "ledring.h"
#include "network_cfg.h"
#include "aml_playermanager.h"
#include "Qlink_API.h"
#include "homeserver.h"
#include "aml_cloudphone_api.h"

static phoneNode_t phoneData;

keyNode_t *node=NULL;
gatpthread_t keyEventTid;

#define KEY_MAX          12
extern int g_cloudphone_session;
typedef struct _keyDataNode_
{
    uint8 keyData[KEY_MAX];
    int32 keyNum;
}keyDataNode_t;

static keyDataNode_t keyDataNode={0};

static void gatKeyNodeInit(keyNode_t *node)
{
    node->type = 0;
	memset(&node->s_keyval[0],0,sizeof(node->s_keyval));
	iofLockInit((lock_t*)&(node->keyLock), KEYID_KEY_SYNC);
	keyDataNode.keyNum = 0;
}

void keyEventHandle(keyNode_t *pKeyNode)
{
	int32 keyType = 0;
	char buf[64];
	int ret, i;
	int temp_volueIndex;
	int keyIndx=0;
	phoneNode_t* p_phoneNode = &phoneData;
	static int configFlag=0;

	if(pKeyNode == NULL)
	{
		LOG(LEVEL_ERROR,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}

	iofLock((lock_t*)&pKeyNode->keyLock);
	keyType = pKeyNode->type;
	iofUnlock((lock_t*)&pKeyNode->keyLock);
	
    LOG(LEVEL_INFO,"%s key type: %d pKeyNode->s_keyval:%s\n",__FUNCTION__, keyType,pKeyNode->s_keyval);
    switch(keyType) 
	#if 0 //for migu pad
	{
			case LONG_PRESS_FLAG:
				if (!strcmp(pKeyNode->s_keyval, "longpressvolumeup"))
				{
					LOG(LEVEL_INFO,"long press volume up.\n");
					ap_config_handle();
				}
				else if (!strcmp(pKeyNode->s_keyval, "longpressvolumedown"))
				{
					LOG(LEVEL_INFO,"long press volume down.\n");
					ap_config_handle();
				}
				else if (!strcmp(pKeyNode->s_keyval, "longpressmute")) 
				{
					ledControl(WHITE_ROLLING,0,0);
					playermanager_play_file("/media/sound/enter_recovery.pcm");
				} 
				else if (!strcmp(pKeyNode->s_keyval, "longpresspause")) 
				{
					ledControl(BLUE_BLINK,0,0);
					playermanager_play_file("/media/sound/wificonfig_start.pcm");
					
					start_config_wifi_with_BT();
				} 
				else 
					LOG(LEVEL_ERROR,"invalid long press %s\n",pKeyNode->s_keyval);
				break;
	
			case SHORT_PRESS_FLAG:
				if (!strcmp(pKeyNode->s_keyval, "volumeup")){
					ledShowVolume(VOLUME_LEVEL_5);
					ap_config_handle();
				}
				else if (!strcmp(pKeyNode->s_keyval, "volumedown")) {
					ledShowVolume(VOLUME_LEVEL_2);
					ap_config_handle();
				}
				else if (!strcmp(pKeyNode->s_keyval, "mute")) {
					ledControl(BLUE_5,5,0);
					playermanager_play_file("/media/sound/micOpen.pcm");
				}
				else if (!strcmp(pKeyNode->s_keyval, "pause")) {
				   ledControl(BLUE_5,0,0);
				}
				else if (!strcmp(pKeyNode->s_keyval, "alink")){
					ledControl(BLUE_BLINK,60,0);
					playermanager_play_file("/media/sound/andlink_start.pcm");
	
					ap_config_handle();
				}
				else if (!strcmp(pKeyNode->s_keyval, "ftest")) {
					ledControl(WHITE_5,0,0);
				}
				else if (!strcmp(pKeyNode->s_keyval, "v+&pause")) {
				}
				else 
					LOG(LEVEL_ERROR,"invalid short press %s.\n",pKeyNode->s_keyval);
				break;
	
			default:
				LOG(LEVEL_ERROR,"invalid event : %d\n", pKeyNode->type );
			break;
		}
	#else
    {
        case LONG_PRESS_FLAG:
			//overflow keydata
			if(keyDataNode.keyNum >= KEY_MAX && ((!strcmp(pKeyNode->s_keyval, "c"))||(!strcmp(pKeyNode->s_keyval, "h"))))
			{
				LOG(LEVEL_ERROR,"invalid long press %s\n",pKeyNode->s_keyval);
				keyDataNode.keyNum = 0;
				memset(&keyDataNode.keyData[0], 0, sizeof(keyDataNode.keyData));
				break;
			}
			//netconfig, mesg + recall key to config
			if(keyDataNode.keyNum == 0 && (strcmp(pKeyNode->s_keyval, "m")==0))
			{
				configFlag=1;
				keyDataNode.keyNum++;
				LOG(LEVEL_ERROR,"m key\n");
				break;
			}
			if(keyDataNode.keyNum == 1 && (configFlag == 1))
			{
				if(strcmp(pKeyNode->s_keyval, "r")==0)
				{
					LOG(LEVEL_ERROR,"bt config\n");
					configFlag=0;
					keyDataNode.keyNum = 0;
					//playermanager_play_file("/media/sound/wificonfig_start.pcm");
					start_config_wifi_with_BT();
				}
				else if(strcmp(pKeyNode->s_keyval, "0")==0)
				{
					LOG(LEVEL_ERROR,"andlink config\n");
					configFlag=0;
					keyDataNode.keyNum = 0;
					ap_config_handle();
				}
				break;
			}
			//push m key before,should clear configFlag and clear keyDataNode number
			if(configFlag!=0)
			{
				configFlag=0;
				keyDataNode.keyNum = 0;
			}
			if(strcmp(pKeyNode->s_keyval, "c")==0)
			{
				LOG(LEVEL_INFO,"begin to call, keynum:%d, key:%s \n",keyDataNode.keyNum, keyDataNode.keyData);
				
				memset(&phoneData.user_phone_num[0], 0, sizeof(phoneData.user_phone_num));
				memcpy(&phoneData.user_phone_num[0], &keyDataNode.keyData[0], keyDataNode.keyNum);
				statusEventNotify(EVT_PHONE_CALLING, (phoneNode_t*)&p_phoneNode);
				keyDataNode.keyNum = 0;
			}
			else if(strcmp(pKeyNode->s_keyval, "h")==0)
			{
				LOG(LEVEL_INFO,"hangup a call, keynum:%d, key:%s \n",keyDataNode.keyNum, keyDataNode.keyData);
				if(1) //calling status
				{
					cloudphone_hangup();
				}
				keyDataNode.keyNum = 0;
				memset(&keyDataNode.keyData[0], 0, sizeof(keyDataNode.keyData));
			}
			else if(strcmp(pKeyNode->s_keyval, "p")==0)
			{
				LOG(LEVEL_INFO,"pickup a tellphone, keynum:%d, key:%s \n",keyDataNode.keyNum, keyDataNode.keyData);
				keyDataNode.keyNum = 0;
				memset(&keyDataNode.keyData[0], 0, sizeof(keyDataNode.keyData));
				if(1) //receive a call status
				{
					gatTimerDel(getPhoneRingTimer());
					usleep(1000 * 1000);
					cloudphonPickup(g_cloudphone_session);
					sai_voip_imp_start();
				}
			}
			else
			{
				keyIndx = keyDataNode.keyNum;
				memcpy(&keyDataNode.keyData[keyIndx], &pKeyNode->s_keyval[0], 1);
				keyDataNode.keyNum++;
				LOG(LEVEL_INFO,"keynum:%d, key:%s \n",keyDataNode.keyNum, keyDataNode.keyData);
			}
			break;

        case SHORT_PRESS_FLAG:
			LOG(LEVEL_ERROR,"SHORT_PRESS_FLAG press %s.\n",pKeyNode->s_keyval);
			break;

        default:
            LOG(LEVEL_ERROR,"invalid event : %d\n", pKeyNode->type );
        break;
    }
	#endif
}

void *keyEventThread(void *arg) 
{
    sigset_t signal_mask;
    int res;
    char* keyVal;
    int key_flag;
	char keyDigital[2]={0};

    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    res = key_eventprocess_init();
    if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) == -1) {
		LOG(LEVEL_ERROR,"SIGPIPE");
    }

    node = (keyNode_t *)iofMalloc(sizeof(keyNode_t));
    if(NULL == node){
        LOG(LEVEL_ERROR,"no memery for keyNode_t.\r\n");
        return NULL;
    }
    gatKeyNodeInit(node);

	#if 0 //for migu pad
    while(1) {
		keyVal = WaitKey(&key_flag);
		LOG(LEVEL_INFO,"key press : %s, %d\n", keyVal, key_flag);

		if (keyVal == NULL) continue;
		if (key_flag == LONG_PRESS_FLAG) {
			iofLock((lock_t*)&node->keyLock);
			node->type = LONG_PRESS_FLAG;
			sprintf(node->s_keyval, "longpress%s", keyVal);
			iofUnlock((lock_t*)&node->keyLock);
		} else if (key_flag == SHORT_PRESS_FLAG){
			iofLock((lock_t*)&node->keyLock);
			node->type = SHORT_PRESS_FLAG;
			sprintf(node->s_keyval, "%s", keyVal);
			iofUnlock((lock_t*)&node->keyLock);
		} else {
			LOG(LEVEL_ERROR,"key press flag invalid!\n");
			continue;
	    } 
        statusEventNotify(EVT_KEY, (keyNode_t*)&node);
    }
	#else //for zhaoge pad
    while(1) {
		keyVal = WaitKey(&key_flag);
		LOG(LEVEL_INFO,"key press : %s, %d\n", keyVal, key_flag);

		if (keyVal == NULL) continue;

		if(key_flag == LONG_PRESS_FLAG || key_flag == SHORT_PRESS_FLAG) 
		{
		    if(strcmp("one", keyVal) == 0)
		    {
		    	keyDigital[0]='1';
				LOG(LEVEL_INFO,"LINE: %d, ONE KEY\n", __LINE__);
			}
			else if(strcmp("two", keyVal) == 0)
			{
				keyDigital[0]='2';
				LOG(LEVEL_INFO,"LINE: %d, TWO KEY\n", __LINE__);
			}
			else if(strcmp("three", keyVal) == 0)
			{
				keyDigital[0]='3';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("four", keyVal) == 0)
			{
				keyDigital[0]='4';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("five", keyVal) == 0)
			{
				keyDigital[0]='5';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("six", keyVal) == 0)
			{
				keyDigital[0]='6';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("seven", keyVal) == 0)
			{
				keyDigital[0]='7';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("eight", keyVal) == 0)
			{
				keyDigital[0]='8';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("nine", keyVal) == 0)
			{
				keyDigital[0]='9';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("zero", keyVal) == 0)
			{
				keyDigital[0]='0';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("call", keyVal) == 0)
			{
				keyDigital[0]='c';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("hangup", keyVal) == 0)
			{
				keyDigital[0]='h';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("mesg", keyVal) == 0)
			{
				keyDigital[0]='m';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("recall", keyVal) == 0)
			{
				keyDigital[0]='r';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else if(strcmp("pickup", keyVal) == 0)
			{
				keyDigital[0]='p';
				LOG(LEVEL_INFO,"LINE: %d \n", __LINE__);
			}
			else
			{
				LOG(LEVEL_ERROR,"key press invalid!\n");
				continue;
			}
			iofLock((lock_t*)&node->keyLock);
			node->type = LONG_PRESS_FLAG;
			memset(&node->s_keyval[0], 0, sizeof(node->s_keyval));
			memcpy(&node->s_keyval[0], &keyDigital[0], 1);
			//sprintf(node->s_keyval, "%s", keyVal);
			iofUnlock((lock_t*)&node->keyLock);
		} 
		else 
		{
			LOG(LEVEL_ERROR,"key press flag invalid!\n");
			continue;
		} 
        statusEventNotify(EVT_KEY, (keyNode_t*)&node);
    }
	#endif
}

void key_event_handle_start(void)
{
	int iRet;
	iRet = iofPthreadCreate(&keyEventTid, NULL, keyEventThread, NULL);
	if(iRet != 0){
		LOG(LEVEL_ERROR,"can't creat keyEventThread. err:%s\r\n", strerror(iRet));
	}else{
		LOG(LEVEL_INFO, "key event handlet thread start...");
	}
	return ;
}

