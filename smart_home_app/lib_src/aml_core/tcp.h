/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _SOCKET_TCP_H_
#define _SOCKET_TCP_H_
#include "gattypes.h"
#include "gatlist.h"
#include "statusevent_manage.h"

#define TCP_NR_MAX          30

typedef struct _tcpSendListNode_
{
    struct gatListHead list;
    int32 fd;
    uint8* data;
    int32 len;
    uint8* arg;
}tcpSendListNode_t;

typedef struct _tcpSendListNr_
{
    lock_t sendLock;    
    struct gatListHead tcpSendList;       
    //struct gatListHead tcpSendingList;     
    int32 sendNr;          
    int32 sendNrMax;
    int32 beatCnt;          
}tcpSendListNr;

void tcpSendInit(void);
int32 gatTcpSend(int32 fd, uint8 *data, int32 dataLen, void *arg); //int32 timeoutSec, gatTcpSendCb cb, 
int32 tcpSendHandle(int32 fd);
int32 gatCloseTcpSocket(int32 fd);
tcpSendListNr* getTcpSendListNr(void);
int32 heartBeatDecrease(void);
#endif /* _SOCKET_TCP_H_ */

