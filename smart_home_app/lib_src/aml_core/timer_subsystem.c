/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "timer_subsystem.h"
#include "gattypes.h"

int32  gatTimerTraversal(void);
static int32  gatTimerForward(gatTimer_st* pForwordTimer, uint32 fwTime);
static int32  gatTimerChangePeriod(gatTimer_st* pChangeTimer, uint32 cPeriod);
int32  gatTimerTick(uint32 dTime);

static int32  timerNodeInit(gatHalTimer_st *pTimer,
                                char* szName,
                                uint32 expire,
                                timerCB functionCB,
                                void* param);
static int32  timerRemove(gatHalTimer_st* pTimer);
static int32  timerDetectTrigger(void);
static int32  timerUpdataExpire(void);
static int32  timerLoop(uint8 flag);
static int32  timerInsert(gatHalTimer_st *pTimer);
static pAgentTimerCore_st  getTimerCenter(void);
static int32  timerTriggerNotifyLp(void* pPrama);
static int32  timerTriggerNotifyHp(void* pPrama);

agentTimerCore_st gatTimerCore;

void  gatTimerCoreInit()
{
    iofLockInit( &gatTimerCore.lock, KEYID_TIMERSYSTEM_SYNC);
    gatTimerCore.count = 0;
    gatTimerCore.expose = 0;
    gatListHeadInit(&(gatTimerCore.timerList));
    timerPrintf( GAT_WARNING,"%s ...... ok\n",__FUNCTION__ );
}

int32  gatTimerAdd(gatTimer_st *pTimer,
                    char* szName,
                    uint32 expire,
                    timerCB functionCB,
                    void* param)
{
    struct gatListHead *pListNode=NULL;
    gatHalTimer_st *pos = NULL;
    gatHalTimer_st *pNodeEntry=NULL;

    iofLock(&gatTimerCore.lock);

    gatListForEachEntry( pListNode, &(gatTimerCore.timerList), entry)
    {
        pos = gatListEntry( pListNode,gatHalTimer_st,entry );
        if( pos->appTimer == pTimer )
        {
            /* remove node from list */
            timerRemove(pos);
            pNodeEntry = pos;
            timerPrintf( GAT_NOTIC,"Timer:%s had in TimerList\n", szName );
            break;
        }
    }
    if( NULL==pNodeEntry )
    {
        pNodeEntry = (gatHalTimer_st*)iofMalloc( sizeof(gatHalTimer_st) );
        timerPrintf( GAT_DEBUG,"%s %d pNodeEntry:%p\n",__FUNCTION__,__LINE__,pNodeEntry );
        if( NULL==pNodeEntry )
        {
            timerPrintf( GAT_WARNING,"%s %d malloc gatHalTimer_st Fail!\n",__FUNCTION__,__LINE__ );
            iofUnlock(&gatTimerCore.lock);
            return -1;
        }
        iofMemset(pNodeEntry,0,sizeof(gatHalTimer_st));
        pNodeEntry->appTimer = pTimer;
    }

    if(expire + gatTimerCore.expose >= TIMER_EXPIRE_MAX)
    {
        timerUpdataExpire();
    }
    timerNodeInit(pNodeEntry, szName, expire, functionCB, param);
    timerInsert(pNodeEntry);

    //unlock
    timerPrintf(GAT_DEBUG,"add timer:%s ok.\r\n", pTimer->szName);
    iofUnlock(&gatTimerCore.lock);
    //gatTimerTraversal();
    return 0;
}

static int32  timerNodeInit( gatHalTimer_st *pTimer,
                                char* szName,
                                uint32 expire,
                                timerCB functionCB,
                                void* param)
{
    pTimer->entry.prev = &pTimer->entry;
    pTimer->entry.next = &pTimer->entry;
    pTimer->expire = expire + gatTimerCore.expose;

    pTimer->appTimer->szName = (szName != NULL) ? szName : "Timer";
    pTimer->appTimer->period = expire; 
    pTimer->appTimer->timerCB = functionCB;
    pTimer->appTimer->param = param;

    timerPrintf(GAT_DEBUG,"Timer: %s init. \r\n",pTimer->appTimer->szName );
    return 0;
}

int32  gatTimerDel(gatTimer_st* pTimer)
{
    gatHalTimer_st *pos=NULL,*pNodeEntry=NULL;
    struct gatListHead *pListNode=NULL,*pListNodeNext=NULL;
    iofLock(&gatTimerCore.lock);
    gatListForEachEntrySafe(pListNode, pListNodeNext, &(gatTimerCore.timerList), entry)
    {
        pos = gatListEntry( pListNode,gatHalTimer_st,entry );
        if( pos->appTimer == pTimer )
        {
            timerPrintf(GAT_DEBUG,"del timer:%s ok\r\n", pos->appTimer->szName);
            timerRemove(pos);
            iofFree((char*)pos);
            iofUnlock(&gatTimerCore.lock);
            return GAT_OK;
        }
    }
    iofUnlock(&gatTimerCore.lock);
    return GAT_ERR_NORES;
}

static int32  timerRemove(gatHalTimer_st* pTimer )
{
    gatListDel(&(pTimer->entry));
    gatTimerCore.count--;
    return 0;
}

int32  gatTimerTick(uint32 dTime)
{
    int32 trigger = 0;
    gatHalTimer_st *cur,*next;
    struct gatListHead *pListNode=NULL,*pListNodeNext=NULL;
    iofLock(&gatTimerCore.lock);
    gatTimerCore.expose += dTime;
    if(gatTimerCore.expose >= TIMER_EXPOSE_MAX)
    {
        timerUpdataExpire();
    }

    gatListForEachEntrySafe(pListNode, pListNodeNext, &gatTimerCore.timerList, entry)
    {
        cur = gatListEntry( pListNode,gatHalTimer_st,entry );
        if(gatTimerCore.expose >= cur->expire)
        {
            timerRemove(cur);
            timerTriggerNotifyLp(cur);
        }
        else
        {
            break;
        }
    }

    iofUnlock(&gatTimerCore.lock);
    return 0;
}

static int32  timerUpdataExpire()
{
    struct gatListHead *pListNode=NULL;
    gatHalTimer_st *pos = NULL;

    /* don't need
    if(gatListEmpty(&(gatTimerCore.timerList)))
    {
        gatTimerCore.expose = 0;
        gatTimerCore.count = 0;
        return 0;
    }
    */
    gatListForEachEntry(pListNode, &(gatTimerCore.timerList), entry)
    {
        pos = gatListEntry( pListNode,gatHalTimer_st,entry );
        if(gatTimerCore.expose >= pos->expire)
        {
            pos->expire = 0;
        }
        else
        {
            pos->expire -= gatTimerCore.expose;
        }
    }
    gatTimerCore.expose = 0;
    timerPrintf(GAT_DEBUG,"Updata timer list expire.\r\n");
    return 1;
}

static int32  timerInsert(gatHalTimer_st *pTimer)
{
    gatHalTimer_st *pos = NULL;
    struct gatListHead *pListNode=NULL;

    gatListForEachEntryReverse(pListNode, &(gatTimerCore.timerList), entry)
    {
        pos = gatListEntry( pListNode,gatHalTimer_st,entry );
        if(pTimer->expire >= pos->expire)
        {
            break;
        }
    }
    gatListAdd(&(pTimer->entry), pListNode/*&(pos->entry)*/);

    gatTimerCore.count++;
    return 0;
}

int32  gatTimerTraversal(void)
{
    struct gatListHead *pListNode=NULL;
    gatHalTimer_st *pos = NULL;

    iofLock(&gatTimerCore.lock);

    if(gatListEmpty(&(gatTimerCore.timerList)))
    {
        gatTimerCore.expose = 0;
        gatTimerCore.count = 0;
        iofUnlock(&gatTimerCore.lock);
        return 0;
    }
    timerPrintf(GAT_DEBUG,"-------------------------Timer list----------------------------------------------\r\n");
    timerPrintf(GAT_DEBUG,"expire         period         expose         addrPrev         addrNext         addrNode         name\r\n");
    gatListForEachEntry(pListNode, &(gatTimerCore.timerList), entry)
    {
        pos = gatListEntry( pListNode,gatHalTimer_st,entry );
        timerPrintf(GAT_DEBUG,"%-14d %-14d %-14d %-16p %-16p %-16p %s\r\n",pos->expire,pos->appTimer->period,gatTimerCore.expose,pos->entry.prev,pos->entry.next,pos,pos->appTimer->szName);
    }
    timerPrintf(GAT_DEBUG,"---------------------------------------------------------------------------------\r\n");
    iofUnlock(&gatTimerCore.lock);
    return 1;
}

int32  timerTriggerNotifyLp(void* pPrama)
{
	uint32 evt = EVT_AGENT_TIMER;
    gatHalTimer_st *pFirstTimer=pPrama;
	gatTimer_st *appTimer = pFirstTimer->appTimer;		  
	
    timerPrintf( GAT_DEBUG,"timer:%s arrive.\n",pFirstTimer->appTimer->szName );
    timerPrintf( GAT_DEBUG,"fun:%s, line:%d, halTimer addr:%p, appTimer addr:%p\n",__FUNCTION__,__LINE__, pFirstTimer, appTimer);
    timerPrintf( GAT_DEBUG,"addr:%p.\n",(void**)&(pFirstTimer->appTimer) );
	
	appTimer->addr = (void*)pFirstTimer; 
	statusEventNotify(evt, (void**)&(pFirstTimer->appTimer)); 
    return 0;
}

int32 gatTimerHandle(gatTimer_st** appTimer) 
{
	int32 ret;
    gatHalTimer_st *pNodeEntry=NULL;
    gatTimer_st *tempTimer=*appTimer;
	int8* pAddr=tempTimer->addr;


    pNodeEntry = gatListEntry(appTimer, gatHalTimer_st, appTimer );
    timerPrintf( GAT_NOTIC,"%s %d p=%p, pAddr:%p\n",__FUNCTION__,__LINE__,pNodeEntry, pAddr );
    ret = pNodeEntry->appTimer->timerCB( pNodeEntry->appTimer );
    // ret = pSig->val.appTimerVal.timerCB( *(pSig->val.appTimerVal.param) );
    if( ret<=0 )
    {
        timerPrintf( GAT_NOTIC,"timer:%s del now.\n",pNodeEntry->appTimer->szName );
    }
    else
    {
        // tempTimer = pNodeEntry->appTimer;
        // tempTimer = ((gatTimer_st*)(*(pSig->val.appTimerVal.param)));//->period = ret;
        tempTimer = pNodeEntry->appTimer;
        tempTimer->period=ret;
        timerPrintf( GAT_NOTIC,"%s %d timer:%s add period=%d\n",__FUNCTION__,__LINE__,tempTimer->szName,ret );
        gatTimerAdd( tempTimer, tempTimer->szName, tempTimer->period, tempTimer->timerCB, tempTimer->param);
    }
    //iofFree( (char*)pNodeEntry );
	iofFree( (char*)pAddr );
	return 0;
}
