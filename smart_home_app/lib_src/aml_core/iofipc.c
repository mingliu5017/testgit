/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "gatplatform.h"
#include "iofipc.h"
#include "semaph.h"

static int32 gatMsgId=-1;

int32 getMsgControlId( void )
{
	return gatMsgId;
}

int32 iofMsgCreat(int32 interKeyId)
{
    int32 keyId;

    keyId = ftok(GAT_PATH_NAME, interKeyId);
    if(keyId < 0)
    {
        printf("ftok keyid fail.err:%d\r\n", keyId);
        return -1;
    }
    gatMsgId = msgget(keyId, 0666 | IPC_CREAT);
    if(gatMsgId < 0)
    {
        printf("msgget fail.err:%d\r\n", gatMsgId);
        return -1;
    }

    return 0;
}


int32 iofSendSig(int32 taskId, iofSig_t *sig)
{
    int32 iRet;
    msgData_t msg;

    msg.msgType = 1;
    memcpy(msg.data, sig, sizeof(iofSig_t));
    //printf("msg send:%d\r\n",sig->evtSigVal.event);
    //dumpBuf( GAT_DEBUG,msg.data, sizeof(iofSig_t) );

    iRet = msgsnd(taskId, (void*)&msg, sizeof(iofSig_t), 0);
    if(iRet < 0)
    {
        return GAT_ERR_FAIL;
    }

    return GAT_OK;
}

int32 iofRecSig(int32 taskId, iofSig_t *sig)
{
    msgData_t msg;
    int32 len;

    len = msgrcv(taskId, (void *)&msg, sizeof(iofSig_t), 0, 0);
    if(len > 0)
    {
        memcpy((void *)sig, (void *)msg.data, len);
    }

    return len;
}

int32 iofSemInit(lock_t *lock, int32 initVal, int32 semSubId)
{
    int32 semId;
    int32 keyId;

    keyId = ftok(GAT_PATH_NAME, semSubId);
    if(keyId < 0)
    {
        printf("init lock fail\r\n");
        return -1;
    }
    semSubId++;

    semId = semCreate(keyId, initVal);
    if(semId < 0)
    {
        printf("init lock fail\r\n");
        return -1;
    }

    *lock = semId;

    return 0;
}


int32 iofLockInit(lock_t *lock, int32 subId)
{
    iofSemInit(lock, 1, subId);

    return 0;
}
int32 iofLock(lock_t *lock)
{
    if(NULL != lock)
        semOp(*lock, -1);
    return 1;
}
int32 iofUnlock(lock_t *lock)
{
    if(NULL != lock)
    semOp(*lock, 1);
    return 1;
}

void statusEventNotify(uint32 event, void *param)
{
	iofSig_t sig;
	sig.cmd = SIG_CMD_EVT;
	sig.evtSigVal.event = event;
	if(param ==  NULL)
	{
		sig.evtSigVal.param = 0;
	}
	else
	{
        sig.evtSigVal.param = *(uint32 *)param;
        printf("iofipc statusEventNotify param addr:%p \r\n", *(uint32*)param);
		
	}
	iofSendSig(getMsgControlId(), &sig);
}


