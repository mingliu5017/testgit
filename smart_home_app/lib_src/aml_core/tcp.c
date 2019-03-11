/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "gatplatform.h"
#include "tcp.h"
#include "ioftcp.h"
#include "gatlist.h"

/* tcpSendNr */
static tcpSendListNr tcpSendNr;

tcpSendListNr* getTcpSendListNr(void)
{
	return &tcpSendNr;
}

void tcpSendInit(void)
{
    /* tcpSend */
    iofLockInit(&tcpSendNr.sendLock, KEYID_TCPSND_SYNC);
    gatListHeadInit(&tcpSendNr.tcpSendList);
    tcpSendNr.sendNr =0;
    tcpSendNr.sendNrMax = TCP_NR_MAX;
    tcpSendNr.beatCnt = 0;
}

static void gatTcpSendNodeInit(tcpSendListNode_t *node)
{
    node->fd = -1;
    gatListHeadInit(&node->list);
    node->data = NULL;
    node->arg = NULL;
    node->len = 0;
}

int32 gatTcpSend(int32 fd, uint8 *data, int32 dataLen,void *arg)//int32 timeoutSec, gatTcpSendCb cb
{
	uint32 evt = EVT_TCP_SEND;
    tcpSendListNode_t* node = NULL;
	
    if(fd < 0 || NULL == data || dataLen <= 0)
    {
        tcpPrintf( GAT_ERROR,"param err!!!");
        return GAT_ERR_FAIL;
    }
    iofLock(&tcpSendNr.sendLock);
    if(tcpSendNr.sendNr >= tcpSendNr.sendNrMax)
    {
        tcpPrintf( GAT_ERROR,"tcp no res.nr:%d,max:%d\r\n", tcpSendNr.sendNr, tcpSendNr.sendNrMax);
        iofUnlock(&tcpSendNr.sendLock);
        return GAT_ERR_NORES;
    }
    iofUnlock(&tcpSendNr.sendLock);
    node = (tcpSendListNode_t *)iofMalloc(sizeof(tcpSendListNode_t));

    if(NULL == node)
    {
        tcpPrintf( GAT_ERROR,"no mem for malloc tcp send node\r\n");
        return GAT_ERR_NORES;
    }
    gatTcpSendNodeInit(node);
    node->fd= fd;
    node->arg= arg;
    node->len= dataLen;
    node->data= (uint8*)iofMalloc(dataLen);
    //tcpPrintf( GAT_DEBUG,"%s,line: %d, data:%s, fd=%d\n",__FUNCTION__,__LINE__,node->data,node->fd );
    if(NULL == node->data)
    {
        tcpPrintf( GAT_ERROR,"malloc tcp send buf fail\r\n");
        iofFree(node);
        return GAT_ERR_NORES;
    }
    iofMemset(node->data,0, dataLen);
    iofMemcpy(node->data, data, dataLen);
    iofLock(&tcpSendNr.sendLock);
    gatListAddTail(&node->list, &tcpSendNr.tcpSendList);
	tcpSendNr.sendNr++;
    iofUnlock(&tcpSendNr.sendLock);
    statusEventNotify(evt, (tcpSendListNode_t*)&node); 
    return GAT_OK;
}

int32 tcpSendHandle(int32 fd)
{
	int32 ret = GAT_OK;
    tcpSendListNode_t* node = NULL;
    struct gatListHead *pListNode=NULL,*pListNodeNext=NULL;
	
    iofLock(&tcpSendNr.sendLock);
	gatListForEachEntrySafe(pListNode, pListNodeNext, &tcpSendNr.tcpSendList, list)
    {
        node = gatListEntry( pListNode,tcpSendListNode_t,list );
		/*delete the invalid socket node */
        if(node->fd < 0)
        {
            tcpPrintf( GAT_WARNING,"invalid fd:%d\r\n", node->fd);
            gatListDel(&node->list);
            if(node->data)
            {
                iofFree(node->data);
                node->data = NULL;
            }
            iofFree((uint8 *)node);
            tcpSendNr.sendNr--;
            continue;
        }
		/*find the fd which is intend to send */
        if(fd == node->fd)
        {
            if((node->data== NULL) || (node->len <= 0))
            {
				tcpPrintf( GAT_WARNING,"invalid send data\r\n");
				gatListDel(&node->list);
				if(node->data)
				{
					iofFree(node->data);
					node->data = NULL;
				}
				iofFree((uint8 *)node);
				tcpSendNr.sendNr--;
				continue;
            }
			//tcpPrintf( GAT_DEBUG,"%s fd=%d data:%p len =%d \r\n",__FUNCTION__,node->fd,node->data,node->len );
			tcpPrintf( GAT_DEBUG,"%s fd=%d arg:%p send %d bytes, ret:%d \r\n",__FUNCTION__,node->fd,node->arg,node->len, ret);
			//dumpBuf( GAT_DEBUG,node->data, node->len );	
			ret = gatIofTcpSend( node->fd, node->data, node->len );
			gatListDel(&node->list);
			tcpSendNr.sendNr--;
			if(node->data)
			{
				iofFree(node->data);
				node->data = NULL;
			}
			iofFree((uint8 *)node);
			if( ret<0 )
			{
				iofUnlock(&tcpSendNr.sendLock);
				return GAT_ERR_FAIL;
			}	
        }
    }
    iofUnlock(&tcpSendNr.sendLock);
    return GAT_OK;
}

/*Ö÷¶¯¹Ø±Õtcp socket */
int32 gatCloseTcpSocket(int32 fd)
{
	int32 ret = GAT_OK;
	tcpSendListNode_t* node = NULL;
	struct gatListHead *pListNode=NULL,*pListNodeNext=NULL;
	
	iofLock(&tcpSendNr.sendLock);
	gatListForEachEntrySafe(pListNode, pListNodeNext, &tcpSendNr.tcpSendList, list)
	{
		node = gatListEntry( pListNode,tcpSendListNode_t,list );
		/*delete the invalid socket node */
		if(node->fd < 0)
		{
			tcpPrintf( GAT_WARNING,"invalid fd:%d\r\n", node->fd);
			gatListDel(&node->list);
			if(node->data)
			{
				iofFree(node->data);
				node->data = NULL;
			}
			iofFree((uint8 *)node);
			tcpSendNr.sendNr--;
			continue;
		}
		/*find the fd which is intend to send */
		if(fd == node->fd)
		{
			if((node->data== NULL) || (node->len <= 0))
			{
				tcpPrintf( GAT_WARNING,"invalid send data\r\n");
				gatListDel(&node->list);
				if(node->data)
				{
					iofFree(node->data);
					node->data = NULL;
				}
				iofFree((uint8 *)node);
				tcpSendNr.sendNr--;
				continue;
			}
			//tcpPrintf( GAT_DEBUG,"%s fd=%d data:%p len =%d \r\n",__FUNCTION__,node->fd,node->data,node->len );
			tcpPrintf( GAT_DEBUG,"%s fd=%d arg:%p send %d bytes, ret:%d \r\n",__FUNCTION__,node->fd,node->arg,node->len, ret);
			//dumpBuf( GAT_DEBUG,node->data, node->len );
			gatIofSocketClose(fd);
			gatListDel(&node->list);
			tcpSendNr.sendNr--;
			if(node->data)
			{
				iofFree(node->data);
				node->data = NULL;
			}
			iofFree((uint8 *)node);
		}
	}
	iofUnlock(&tcpSendNr.sendLock);
	return GAT_OK;
}


int32 heartBeatDecrease(void)
{
	int32 ret = GAT_OK;
    iofLock(&getTcpSendListNr()->sendLock);
	if(tcpSendNr.beatCnt > 0)
	{
		tcpSendNr.beatCnt--;
		tcpPrintf( GAT_DEBUG,"beatCnt: %d!\r\n", tcpSendNr.beatCnt);
	}
	else
	{
		ret = GAT_ERR_FAIL;
		tcpPrintf( GAT_ERROR,"beatCnt(%d) invalid!\r\n", tcpSendNr.beatCnt);
	}
    iofUnlock(&getTcpSendListNr()->sendLock);
	return ret;
}

int32 heartBeatClear(void)
{
	int32 ret = GAT_OK;
    iofLock(&getTcpSendListNr()->sendLock);
	tcpSendNr.beatCnt=0;
    iofUnlock(&getTcpSendListNr()->sendLock);
	return ret;
}

int32 checkBeatCnt(void)
{
	int32 ret = 0;
    iofLock(&getTcpSendListNr()->sendLock);
	ret = getTcpSendListNr()->beatCnt++;
	gatPrintf(GAT_DEBUG,"%s beatCnt:%d \r\n",__FUNCTION__, getTcpSendListNr()->beatCnt);
	iofUnlock(&getTcpSendListNr()->sendLock);
	return ret;
}


