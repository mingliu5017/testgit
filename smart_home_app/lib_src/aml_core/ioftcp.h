/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _IOF_TCP_H
#define _IOF_TCP_H
#include "gattypes.h"

#define TCP_SKTBUF_SIZE         4096   
#define TCP_LISTEN_MAX          8

typedef void (*gatTcpConnCb)(int32 fd, int32 result);

void gatIofSocketClose(int32 fd);
int32 gatIofSocketOpen(void);
int32 gatIofSocketConnect(int32 fd, int8 *szIp, int32 port, gatTcpConnCb cb);
int32 gatIofSelect(int32 nfds, gat_fd_set *readfds, gat_fd_set *writefds,gat_fd_set *exceptfds,int32 timeout_sec,int32 timeout_usec );
int32 gatIofTcpSend(int32 fd, int8 *data, int32 dataLen);
int32 gatIofSocketUdpRecv( int32 sockfd, void *buf, int32 len, int32 flags,gatSockAddr *src_addr, int32 *addrlen );
int32 gatIofSocketTcpRecv( int32 sockfd, void *buf, int32 len, int32 flags );

int32 gatIofGetHostByName( int8 *domain, uint8 *strIp);
int32 gatIofTcpCreateServer(uint16 port);
int32 gatIofTcpServerListen(int32 fd);

#endif /* _IOF_TCP_H */
