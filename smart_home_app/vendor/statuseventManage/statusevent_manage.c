/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "gatgobal.h"
#include "statusevent_manage.h"
#include "tcp.h"
#include "homeserver.h"
#include "timer_subsystem.h"
#include "key_event_handler.h"

static devStatus_st devStatus;
static wakeupNode_t wakeupNode;

static recNode_t* recNode = NULL; 

static void evtSetDevStatus(uint32 status,int8 flag );
static void evtClearDevStatus(void);
static void networkEventHandle(int* change);
static void statusSwitchHandle(int* source); 

void evtDevStatusInit(void)
{
	memset(&devStatus,0,sizeof(devStatus_st));
    iofLockInit(&devStatus.devLock, KEYID_DEVSTATUS_SYNC);
	evtClearDevStatus();
}
/*
return 0:disconnect, 1:connected
*/
int32 isNetConnected(void)
{
	uint32 curStatus = 0;
	curStatus = getDevStatus(); 
	if((curStatus&DEV_STATUS_CONNECTED)==DEV_STATUS_CONNECTED)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
return 1:cloud phone busy, 0:idle/free
*/
int32 isCloudPhoneBusy(void)
{
	if(	 getDevStatus()&DEV_STATUS_TALKING||
		(getDevStatus()&DEV_STATUS_CALLING)||
		(getDevStatus()&DEV_STATUS_RINGING))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
return 1:bt phone busy, 0:idle/free
*/
int32 isBtPhoneBusy(void)
{
	if(getDevStatus()&DEV_STATUS_BTPHONE_RINGING||getDevStatus()&DEV_STATUS_BTPHONE_CALLING||getDevStatus()&DEV_STATUS_BTPHONE_TALKING)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
return 0:music,1:book,2:text,3:radio\story...,4:bt music
       5:restaurant,6:news,7:joke,8:poetry.
*/
int32 getPlayType(void)
{
	if(getDevStatus() & DEV_STATUS_PLAYMUSIC)
	{
		return PLAYTYPE_MUSIC;
	}
	else if(getDevStatus() & DEV_STATUS_PLAYBOOK)
	{
		return PLAYTYPE_BOOK;
	}
	else if(getDevStatus() & DEV_STATUS_PLAYTEXT)
	{
		return PLAYTYPE_TEXT;
	}
	else if(getDevStatus() & DEV_STATUS_PLAYMEDIA)
	{
		return PLAYTYPE_MEDIA;
	}
	else if(getDevStatus() & DEV_STATUS_PLAYBLUETOOTH)
	{
		return PLAYTYPE_BLUETOOTH;
	}
	else if(getDevStatus() & DEV_STATUS_PLAYRESTAURANT)
	{
		return PLAYTYPE_RESTAURANT;
	}
	else if(getDevStatus() & DEV_STATUS_PLAYNEWS)
	{
		return PLAYTYPE_NEWS;
	}
	else if(getDevStatus() & DEV_STATUS_JOKE)
	{
		return PLAYTYPE_JOKE;
	}
	else if(getDevStatus() & DEV_STATUS_POETRY)
	{
		return PLAYTYPE_POETRY;
	}
	else
	{
		return -1;
	}
}



void setDevStatus(uint32 status, int8 trueFlag)
{
	uint32 curStatus = 0;

	switch(status)
	{
		case DEV_STATUS_MUTE_PENDING:
			if(trueFlag == 1)
			{
				curStatus = DEV_STATUS_MASK & DEV_STATUS_MUTE_PENDING;
				evtSetDevStatus(curStatus, 1);
			}
			else
			{
				curStatus = DEV_STATUS_MASK & DEV_STATUS_MUTE_PENDING;
				evtSetDevStatus(curStatus, 0);
			}
			break;
		case DEV_STATUS_IVW_ACKING:
			if(trueFlag == 1)
			{
				curStatus = DEV_STATUS_MASK & DEV_STATUS_IVW_ACKING;
				evtSetDevStatus(curStatus, 1);
			}
			else
			{
				curStatus = DEV_STATUS_MASK & DEV_STATUS_IVW_ACKING;
				evtSetDevStatus(curStatus, 0);
			}
			break;
		case DEV_STATUS_MOBAIHE_MODE: 
			if(trueFlag == 1)
			{
				curStatus = DEV_STATUS_MASK&(DEV_STATUS_PLAYING|DEV_STATUS_PAUSE|DEV_STATUS_STOP|DEV_STATUS_PLAYMUSIC|DEV_STATUS_PLAYBOOK|
											DEV_STATUS_PLAYMEDIA | DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYBLUETOOTH|
											DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE|DEV_STATUS_MOBAIHE_SEARCH|DEV_STATUS_POETRY);
				evtSetDevStatus(curStatus, 0);
				curStatus = DEV_STATUS_MASK&DEV_STATUS_MOBAIHE_MODE;
				evtSetDevStatus(curStatus, 1);
			}
			else
			{
				curStatus = DEV_STATUS_MASK&DEV_STATUS_MOBAIHE_MODE;
				evtSetDevStatus(curStatus, 0);
			}
			break;
		case DEV_STATUS_MOBAIHE_SEARCH: 
			if(trueFlag == 1)
			{
				curStatus = DEV_STATUS_MASK&DEV_STATUS_MOBAIHE_MODE;
				evtSetDevStatus(curStatus, 0);
				curStatus = DEV_STATUS_MASK&DEV_STATUS_MOBAIHE_SEARCH;
				evtSetDevStatus(curStatus, 1);
			}
			else
			{
				curStatus = DEV_STATUS_MASK&DEV_STATUS_MOBAIHE_SEARCH;
				evtSetDevStatus(curStatus, 0);
			}
			break;
		case DEV_STATUS_IDLE:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYING|DEV_STATUS_PAUSE|DEV_STATUS_STOP|DEV_STATUS_PLAYMUSIC|DEV_STATUS_PLAYBOOK|
										DEV_STATUS_PLAYMEDIA | DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYBLUETOOTH|
										DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK&DEV_STATUS_IDLE;
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_IDLE;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PLAYING:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE|DEV_STATUS_STOP );
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK&DEV_STATUS_PLAYING;
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_PLAYING;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PLAYMUSIC:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_IDLE | DEV_STATUS_PAUSE | DEV_STATUS_PLAYBOOK | DEV_STATUS_STOP|
										DEV_STATUS_PLAYMEDIA | DEV_STATUS_PLAYTEXT | DEV_STATUS_PLAYBLUETOOTH|
										DEV_STATUS_PLAYRESTAURANT | DEV_STATUS_PLAYNEWS | DEV_STATUS_JOKE | DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYING | DEV_STATUS_PLAYMUSIC) ;
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYMUSIC);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PLAYBOOK:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE | DEV_STATUS_PLAYMUSIC |DEV_STATUS_STOP|
										DEV_STATUS_PLAYMEDIA | DEV_STATUS_PLAYTEXT|
										DEV_STATUS_PLAYBLUETOOTH|DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYING | DEV_STATUS_PLAYBOOK);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYBOOK);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PLAYTEXT:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE | DEV_STATUS_PLAYMUSIC |DEV_STATUS_STOP|
										DEV_STATUS_PLAYBOOK | DEV_STATUS_PLAYMEDIA|
										DEV_STATUS_PLAYBLUETOOTH|DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYING |DEV_STATUS_PLAYTEXT);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYTEXT);
			evtSetDevStatus(curStatus, 0);
		}

		break;
	case DEV_STATUS_PLAYMEDIA:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE | DEV_STATUS_PLAYMUSIC |DEV_STATUS_STOP|
										DEV_STATUS_PLAYBOOK | DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYBLUETOOTH|
										DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYING | DEV_STATUS_PLAYMEDIA);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_PLAYMEDIA);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PAUSE:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PLAYING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK&DEV_STATUS_PAUSE;
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_PAUSE;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_STOP:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_STOP;
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_STOP;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_CONFIGWIFI:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_CONFIGWIFI;
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_CONFIGWIFI;
			evtSetDevStatus(curStatus, 0);
		}
		break;

	case DEV_STATUS_CONNECTED:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_CONNECTED;
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK&DEV_STATUS_CONNECTED;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_ALARM:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE );
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_ALARM);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & DEV_STATUS_ALARM;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_TALKING:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_RINGING|DEV_STATUS_HANGUP|DEV_STATUS_CALLING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_TALKING);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & DEV_STATUS_TALKING;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_CALLING:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_RINGING|DEV_STATUS_HANGUP|DEV_STATUS_TALKING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_CALLING);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & DEV_STATUS_CALLING;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_RINGING:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_TALKING|DEV_STATUS_HANGUP|DEV_STATUS_CALLING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_RINGING);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & DEV_STATUS_RINGING;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_HANGUP:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_TALKING|DEV_STATUS_RINGING|DEV_STATUS_CALLING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_HANGUP);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & DEV_STATUS_HANGUP;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_BTPHONE_TALKING:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_BTPHONE_RINGING|DEV_STATUS_BTPHONE_CALLING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_BTPHONE_TALKING);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & (DEV_STATUS_BTPHONE_TALKING|DEV_STATUS_BTPHONE_RINGING|DEV_STATUS_BTPHONE_CALLING);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_BTPHONE_RINGING:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_BTPHONE_TALKING|DEV_STATUS_BTPHONE_CALLING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_BTPHONE_RINGING);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & DEV_STATUS_BTPHONE_RINGING;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_BTPHONE_CALLING:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_BTPHONE_TALKING|DEV_STATUS_BTPHONE_RINGING);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_BTPHONE_CALLING);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & DEV_STATUS_BTPHONE_CALLING;
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_MICMUTE:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_MICMUTE);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_MICMUTE);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PLAYBLUETOOTH:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE|DEV_STATUS_PLAYMUSIC|DEV_STATUS_PLAYBOOK | DEV_STATUS_STOP|
				DEV_STATUS_PLAYMEDIA|DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYING | DEV_STATUS_PLAYBLUETOOTH );
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYBLUETOOTH);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PLAYRESTAURANT:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE|DEV_STATUS_PLAYMUSIC|DEV_STATUS_PLAYBOOK | DEV_STATUS_STOP|
				DEV_STATUS_PLAYMEDIA|DEV_STATUS_PLAYBLUETOOTH|DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYING | DEV_STATUS_PLAYRESTAURANT );
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYRESTAURANT);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_PLAYNEWS:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE|DEV_STATUS_PLAYMUSIC|DEV_STATUS_PLAYBOOK | DEV_STATUS_STOP|
				DEV_STATUS_PLAYMEDIA|DEV_STATUS_PLAYBLUETOOTH|DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_JOKE|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYING | DEV_STATUS_PLAYNEWS );
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYNEWS);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_JOKE:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE|DEV_STATUS_PLAYMUSIC|DEV_STATUS_PLAYBOOK | DEV_STATUS_STOP|
				DEV_STATUS_PLAYMEDIA|DEV_STATUS_PLAYBLUETOOTH|DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYING | DEV_STATUS_JOKE );
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_JOKE);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_POETRY:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK&(DEV_STATUS_IDLE|DEV_STATUS_PAUSE|DEV_STATUS_PLAYMUSIC|DEV_STATUS_PLAYBOOK | DEV_STATUS_STOP|
				DEV_STATUS_PLAYMEDIA|DEV_STATUS_PLAYBLUETOOTH|DEV_STATUS_PLAYTEXT|DEV_STATUS_PLAYRESTAURANT|DEV_STATUS_PLAYNEWS|DEV_STATUS_JOKE);
			evtSetDevStatus(curStatus, 0);
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_PLAYING | DEV_STATUS_POETRY );
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_POETRY);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	case DEV_STATUS_BT_CONNECTED:
		if(trueFlag == 1)
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_BT_CONNECTED);
			evtSetDevStatus(curStatus, 1);
		}
		else
		{
			curStatus = DEV_STATUS_MASK & ( DEV_STATUS_BT_CONNECTED);
			evtSetDevStatus(curStatus, 0);
		}
		break;
	default:
		evtPrintf( GAT_NOTIC,"curStatus:0x%04X, newStatus:0x%04X\n", curStatus, status&DEV_STATUS_MASK);
		break;
	}

	printf("setDevStatus in args    [0x%x],flag:[%d]\n",status,trueFlag);
	printf("setDevStatus new status [0x%x]\n",devStatus.curStatus);
 
    return;
}

/*
    flag=1 set devStatus
    flag=0 reset devStatus 
*/
static void evtSetDevStatus(uint32 status,int8 flag )
{
	iofLock(&devStatus.devLock);
    if(flag==1)
    {
        //devStatus.preStatus = devStatus.curStatus;
		devStatus.curStatus |= status;
    }
    else
    {
        //devStatus.curStatus = devStatus.preStatus;
		devStatus.curStatus &=~ status;
    }
	iofUnlock(&devStatus.devLock);
    return;
}

/*
    clear devStatus ,only used in main thread
*/
static void evtClearDevStatus(void)
{
	iofLock(&devStatus.devLock);
    devStatus.curStatus = DEV_STATUS_CLEAR;
	iofUnlock(&devStatus.devLock);
    return;
}

uint32 getDevStatus(void)
{
	uint32 status = 0;
	iofLock(&devStatus.devLock);
	status = devStatus.curStatus;
	iofUnlock(&devStatus.devLock);
	return status;
}

int32 statusEvtHandle(iofSig_t *sig)
{
    int32 ret = 0;
	evtInfo_st evtInfo ={0};

    evtInfo.event= sig->evtSigVal.event;
    evtPrintf(GAT_NOTIC,"%s event:%d!\r\n",__FUNCTION__,evtInfo.event);
    switch(evtInfo.event)
    {
        case EVT_KEY: 
			keyEventHandle((keyNode_t*)(sig->evtSigVal.param));
			break;
		case EVT_TCP_SEND: 
			tcpEventHandle((tcpSendListNode_t*)(sig->evtSigVal.param));
			break;
		case EVT_IAT_SEMANTIC:
			semanticEventHandle((recNode_t*)(sig->evtSigVal.param));
			break;
		case EVT_MEDIAPLAYER_ACTION:
			break;
		case EVT_MUSICPLAYER_ACTION:
			break;
		case EVT_BOOKPLAYER_ACTION:
			break;
		case EVT_PLAY_TEXTBOOK:
			break;
		case EVT_INTERRUPT: 
			interruptEventHandle((wakeupNode_t*)(sig->evtSigVal.param));
			break;
		case EVT_AGENT_TIMER: 
			if(sig->evtSigVal.param != NULL)
			{			
				evtPrintf( GAT_NOTIC,"param addr:%p\n", sig->evtSigVal.param);
				gatTimer_st **appTimer=NULL;
				appTimer = (gatTimer_st**)&(sig->evtSigVal.param);
				gatTimerHandle(appTimer);
			}
			else
			{
				ret = -1;
			}
			break;
		case EVT_TIMER_ACTION:
				break;
		case EVT_PHONE_CALLING:
			 callingEventHandle((phoneNode_t*)(sig->evtSigVal.param));
				break;
		case EVT_NETWORK_CHANGE:
				networkEventHandle((int*)(sig->evtSigVal.param));
				break;
		case EVT_PLAY_TEXT_END:
			break;
		case EVT_USER_DIALOG:
			break;
		case EVT_PLAYER_SWITCH:
			statusSwitchHandle((int*)(sig->evtSigVal.param));
			break;
	    default:
	        evtPrintf( GAT_NOTIC,"%s  invalid event : %d\n",__FUNCTION__, evtInfo.event );
	        ret = -1;
	        break;
    }
    return ret;
}

int32 wakeupNodeInit(void)
{
    memset(&wakeupNode,0,sizeof(recNode_t));
    iofLockInit(&wakeupNode.wakLock, KEYID_WAKEUP_SYNC);
}

void sendInterruptEvt(int32 type)
{
	int32 evt = EVT_INTERRUPT;
	wakeupNode_t* pWakeupNode = &wakeupNode;
	iofLock(&pWakeupNode->wakLock);
	pWakeupNode->type = type;
	iofUnlock(&pWakeupNode->wakLock);
	evtPrintf( GAT_DEBUG,"addr:%p, type:%d\n",(wakeupNode_t*)&pWakeupNode, type);
	statusEventNotify(evt, (wakeupNode_t*)&pWakeupNode); 
}

static void networkEventHandle(int* change) 
{
	int network_change_event=-1;
	int32 type;
	if(change == NULL)
	{
		evtPrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}
	network_change_event = (int)change;
	evtPrintf( GAT_NOTIC,"%s  network_change_event: %d\n",__FUNCTION__, network_change_event);
	switch (network_change_event)
	{
		case WIFICONFIG_GOT_PASSWORD:
			break;
		case WIFICONFIG_CONNECT_FAIL:
			setDevStatus(DEV_STATUS_CONFIGWIFI, 0);
			break;
		case WIFICONFIG_CONNECT_SUCCESS:
			setDevStatus(DEV_STATUS_CONFIGWIFI, 0);
			break;
		default:
			evtPrintf( GAT_NOTIC,"%s  EVT_NETWORK_CHANGE invalid event : %d\n",__FUNCTION__, network_change_event );
			break;
	}

	return;
}

static void statusSwitchHandle(int* source) 
{
	int32 sourceType=-1;
	int32 type;
	uint32 curStatus;
	if(source == NULL)
	{
		evtPrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}
	curStatus = getDevStatus();
	sourceType = (int32)source;
	evtPrintf( GAT_NOTIC,"%s  sourceType: %d devStatus: 0x%x\n",__FUNCTION__, sourceType,curStatus);
	switch (sourceType)
	{
		case SOURCE_TYPE_BT_PLAY:
			setDevStatus(DEV_STATUS_PLAYBLUETOOTH, 1);
			setDevStatus(DEV_STATUS_PLAYING, 1);
			break;
		case SOURCE_TYPE_GST_PLAY:
			setDevStatus(DEV_STATUS_PLAYING, 1);
			break;
		case SOURCE_TYPE_INSTRUCTION_PAUSE:
			break;
		case SOURCE_TYPE_INSTRUCTION_REPLAY:
			break;
		case SOURCE_TYPE_INSTRUCTION_NEXT:
	    	break;
		case SOURCE_TYPE_INSTRUCTION_PRE:
			break;
		case SOURCE_TYPE_PUSHCONTROLE_START:
			setDevStatus(DEV_STATUS_PLAYING, 1);
			setDevStatus(DEV_STATUS_PLAYMUSIC, 1);
			break;
		case SOURCE_TYPE_PUSHCONTROLE_PAUSE:
			break;
		case SOURCE_TYPE_PUSHCONTROLE_PRE:	
			setDevStatus(DEV_STATUS_PLAYING, 1);
			break;
		case SOURCE_TYPE_PUSHCONTROLE_NEXT:	
			break;
		case SOURCE_TYPE_STOP_BLUETOOTHMUSIC: 		
			break;
		case SOURCE_TYPE_BLUETOOTH_PHONERINGING: 
			setDevStatus(DEV_STATUS_BTPHONE_RINGING, 1);
			break;
		case SOURCE_TYPE_BLUETOOTH_PHONECALLING: 
			setDevStatus(DEV_STATUS_BTPHONE_CALLING, 1);
			break;
		case SOURCE_TYPE_BLUETOOTH_PHONETALKING: 
			break;
		case SOURCE_TYPE_BLUETOOTH_PHONEHUNGUP: 
			break;
		case SOURCE_TYPE_BLUETOOTH_PHONESTOP:
			break;
		case SOURCE_TYPE_BLUETOOTH_PHONE_DISCON: 
			break;
		case SOURCE_TYPE_BLUETOOTH_PHONE_CONNECT:
			setDevStatus(DEV_STATUS_BT_CONNECTED, 1);
			break;
		default:
			evtPrintf( GAT_NOTIC,"%s  sourceType invalid event : %d\n",__FUNCTION__, sourceType );
			break;
	}
}

recNode_t* evt_get_recNode(void)
{
    return recNode;
}

int evt_recNode_init(void)
{
    recNode = (recNode_t*)iofMalloc( sizeof( recNode_t ));
    if(NULL == recNode)
    {
		acePrintf( GAT_ERROR,"recNode malloc error!\r\n");
		return -1;
    }
    memset(recNode,0,sizeof(recNode_t) );
    iofLockInit((lock_t*)&(recNode->recLock), KEYID_REC_SYNC);
	return 0;
}


void semanticEventHandle(recNode_t* pRecNode)
{
	int32 type = 0;
	int8* data = NULL;
	int32 dataLen = 0;
	
	if(pRecNode == NULL)
	{
		acePrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}

	iofLock((lock_t*)&(pRecNode->recLock));
	type = pRecNode->type;
	data = pRecNode->data;
	dataLen = pRecNode->len;
	iofUnlock((lock_t*)&(pRecNode->recLock));
    acePrintf(GAT_DEBUG,"%s, ace type:%d, data:%s \r\n",__FUNCTION__, type, data);

    switch(type)
    {
        case TYPE_SEMANTIC:
            acePrintf( GAT_NOTIC,"TYPE_SEMANTIC!\n" );
			miguSemantic(data,dataLen);
        	break;
		case TYPE_SEMANTIC_FAILED:
 			break;
        default:
            acePrintf( GAT_NOTIC,"%s  invalid event : %d\n",__FUNCTION__, type );
        	break;
    }
}

void interruptEventHandle(wakeupNode_t* pWakeNode)
{
	int32 wakeType=0; 	
	uint32 curStatus=0;
	uint16 action = SPEAKER_ACTION_INVALID;

	if(pWakeNode == NULL)
	{
		acePrintf( GAT_NOTIC,"%s  param NULL : %d\n",__FUNCTION__);
		return;
	}

	iofLock((lock_t*)&(pWakeNode->wakLock));
	wakeType = pWakeNode->type;
	iofUnlock((lock_t*)&(pWakeNode->wakLock));
	evtPrintf( GAT_NOTIC,"interrupt type:%d\n",wakeType);
	curStatus = getDevStatus(); 
	evtPrintf( GAT_NOTIC,"curStatus:0x%04X, play:%d,stop:%d\n", curStatus,(curStatus&DEV_STATUS_PLAYING),(curStatus&DEV_STATUS_STOP));
	//pause
	if((wakeType == TYPE_NOTIFY)&&
			(curStatus&DEV_STATUS_PLAYING)&&!(curStatus&DEV_STATUS_STOP)) 
	{
		action= SPEAKER_ACTION_PAUSE;
	}
	// resume
	else if(wakeType == TYPE_RESULT && (curStatus&DEV_STATUS_PLAYING) && (curStatus&DEV_STATUS_STOP)) 
	{
		if(isCloudPhoneBusy()||isBtPhoneBusy())
		{
			acePrintf( GAT_NOTIC,"telephone busy status\n");
			return;
		}
		else
		{
			action= SPEAKER_ACTION_PLAYING;
		}
	}
    acePrintf(GAT_DEBUG,"%s, action:%d! \r\n",__FUNCTION__, action);

	/*below should do the policy according to devstatus */
    switch(action)
    {
        case SPEAKER_ACTION_PAUSE:
            gatPrintf( GAT_NOTIC,"SPEAKER_ACTION_PAUSE!\n" );
			if(getPlayType()==PLAYTYPE_BLUETOOTH) 
			{
				gatPrintf( GAT_NOTIC,"blue tooth pause!\n" );
				setDevStatus(DEV_STATUS_STOP, 1); 
				//btControlPause();
			} 
			else if(getPlayType()==PLAYTYPE_MUSIC||
					getPlayType()==PLAYTYPE_BOOK||
					getPlayType()==PLAYTYPE_MEDIA||
					getPlayType()==PLAYTYPE_NEWS)
			{
				setDevStatus(DEV_STATUS_STOP, 1); 
				//play_set_pause(0);
			}
			else if(getPlayType()==PLAYTYPE_TEXT){
				setDevStatus(DEV_STATUS_STOP, 1); 
			}
			else
			{
				gatPrintf( GAT_NOTIC,"not supported type:%d!\n",getPlayType());
			}
			
//			if (volueIndex == 0) {
//				system("/etc/adckey/adckey_function.sh homeServerVolume 200");
//				volueIndex = 5;
//			}

        	break;
		case SPEAKER_ACTION_PLAYING:
			gatPrintf( GAT_NOTIC,"SPEAKER_ACTION_PLAYING!\n" );
			if(getPlayType()==PLAYTYPE_BLUETOOTH) 
			{
				gatPrintf( GAT_NOTIC,"blue tooth playing!\n" );
				setDevStatus(DEV_STATUS_STOP, 0);
				//btControlPlay();	
			} 
			else if(getPlayType()==PLAYTYPE_MUSIC||
					getPlayType()==PLAYTYPE_BOOK||
					getPlayType()==PLAYTYPE_MEDIA||
					getPlayType()==PLAYTYPE_NEWS)
			{
				gatPrintf( GAT_NOTIC,"url media playing!\n" );
				setDevStatus(DEV_STATUS_STOP, 0);
				//play_set_pause(1);
			}
			else if(getPlayType()==PLAYTYPE_TEXT){
				gatPrintf( GAT_NOTIC,"text is playing!\n" );
				setDevStatus(DEV_STATUS_STOP, 0);
				bookNode_t *pbookNode =get_bookNode();
				playermanager_play_text(pbookNode->curLine);
			}
			else
			{
				gatPrintf( GAT_NOTIC,"not supported type:%d!\n",getPlayType());
			}
			break;
        default:
            acePrintf( GAT_NOTIC,"%s  invalid player action : %d\n",__FUNCTION__, action );
        	break;
    }
}

