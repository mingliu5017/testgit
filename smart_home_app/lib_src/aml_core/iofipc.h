/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _IOF_IPC_H
#define _IOF_IPC_H

#ifdef __cplusplus
extern "C"{ 
#endif


#include "gatlist.h"
#include "gattypes.h"
//#include "tcp.h"
#include "ioftcp.h"


#define GAT_KEYID_MSG       (0x01)
#define MSG_LEN_MAX         (64)

/* CMD */
#define SIG_CMD_EVT      0  /*event cmd id*/ 
#define SIG_CMD_OTHERS   1 

/*MODULE LOCK KEYID */
#define KEYID_TCPSND_SYNC       (0x00) /*tcp lock key id */
#define KEYID_DEVSTATUS_SYNC      (0x01) /*dev status lock key id */
#define KEYID_REC_SYNC      (0x02) /*rec lock key id */
#define KEYID_WAKEUP_SYNC      (0x03) /*wakeup lock key id */
#define KEYID_TIMERSYSTEM_SYNC      (0x04) /*timer system lock key id */
#define KEYID_KEY_SYNC      (0x05) /*key lock key id */
#define KEYID_TIMER_SYNC      (0x06) /*timer lock key id */
#define KEYID_SONG_SYNC      (0x07) /*song lock key id */
#define KEYID_BOOK_SYNC      (0x08) /*book lock key id */
#define KEYID_CLIENT_SYNC      (0x09) /*client lock key id */

typedef struct
{
	long int msgType; 
    char data[MSG_LEN_MAX];
}msgData_t;

typedef struct _evtSigVal_t_
{
    int32 event;
    void *param;
}evtSigVal_t;

typedef struct
{
    int32 cmd;
    evtSigVal_t evtSigVal;
}iofSig_t;

int32 iofSendSig(int32 taskId, iofSig_t *sig);
int32 iofRecSig(int32 taskId, iofSig_t *sig);
int32 iofLockInit(lock_t *lock, int32 subId);
int32 iofLock(lock_t *lock);
int32 iofUnlock(lock_t *lock);
int32 iofMsgCreat(int32 interKeyId);
int semCreate(key_t, int);
void statusEventNotify(uint32 event, void *param);
int32 getMsgControlId( void );


#ifdef __cplusplus
}
#endif

#endif  /* _IOF_IPC_H */

