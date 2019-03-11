/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "gatplatform.h"
#include "gatgobal.h"
#include "ioftcp.h"
#include "iofipc.h"

void  gatIofSocketClose(int32 fd)
{
    close(fd);
}

int32  gatIofSocketOpen()
{
    int32 sktFd;

    sktFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    return sktFd;
}

int32  gatIofSocketConnect(int32 fd, int8 *szIp, int32 port, gatTcpConnCb cb)
{
    struct sockaddr_in addr;
    int32 iRet;
    int8 msg[32];
    int32 msgLen;
    iofSig_t sig;
    int32 result;

    addr.sin_family = AF_INET;
    addr.sin_port= htons(port);
    addr.sin_addr.s_addr = inet_addr(szIp);
    iRet = connect(fd, (struct sockaddr *)&addr, sizeof( struct sockaddr_in));
    if(iRet < 0)
    {
        iRet = connect(fd, (struct sockaddr *)&addr, sizeof( struct sockaddr_in));
    }
	/*not callback*/
	/*
    if(NULL != cb)
    {
        if(iRet < 0)
        {
            result = GAT_ERR_FAIL;
        }
        else
        {
            result = GAT_OK;
        }
        sig.cmd = SIG_CMD_SKTCON;
        sig.val.conSigVal.fd = fd;
        sig.val.conSigVal.result = result;
        sig.val.conSigVal.cb = cb;
        printf("send conn cb:0x%x,fd:%d\r\n", cb, fd);
        iofSendSig(getMsgControlId(), &sig);
    }
    */
    return iRet;
}

int32  gatIofTcpSend(int32 fd, int8 *data, int32 dataLen)
{
    int32 iRet;

    iRet = send(fd, data, dataLen, 0);

    return iRet;
}
int32  gatIofSocketTcpRecv( int32 sockfd, void *buf, int32 len, int32 flags )
{
    return recv( sockfd,buf,len,flags );
}
int32  gatIofSocketUdpRecv( int32 sockfd, void *buf, int32 len, int32 flags,
                        gatSockAddr *src_addr, int32 *addrlen )
{
    return recvfrom( sockfd,buf,len,flags,src_addr,addrlen );

}

int32  gatIofSelect(int32 nfds, gat_fd_set *readfds, gat_fd_set *writefds,
                  gat_fd_set *exceptfds,int32 timeout_sec,int32 timeout_usec )
{
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = timeout_usec;
    return select( nfds, readfds, writefds,exceptfds, &timeout );
}

int32  gatIofGetHostByName( int8 *domain, uint8 *strIp)
{
    struct hostent *hptr;
    int8   **pptr;
    int8   str[16];

    iofMemset(str, 0x0, sizeof(str));
    hptr = gethostbyname2(domain, AF_INET);
    if (hptr == NULL)
    { 
        return -1;
    }
    pptr=(int8**)hptr->h_addr_list;

    for(; *pptr!=NULL; pptr++)
    {
        inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
    }
    inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str));
    memcpy(strIp, str, 16);

    return 0;
}

int32  gatIofTcpCreateServer(uint16 port)
{
    struct sockaddr_in addr;
    int32 size=0;
    int32 fd=0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if( fd < 0 )
    {
        printf("TCPServer socket create error\n");
        return GAT_ERR_FAIL;
    }
    size = TCP_SKTBUF_SIZE;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, 4);
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, 4);

    size = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const int8 *)&size, sizeof(int32)) < 0)
    {
        printf("socket REUSEADDR err\r\n");
        return GAT_ERR_FAIL;
    }

    iofMemset(&addr, 0x0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if( bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        printf("TCPSrever socket bind error\n");
        gatIofSocketClose(fd);
        return GAT_ERR_FAIL;
    }

    if(listen( fd, TCP_LISTEN_MAX) != 0 )
    {
        printf("TCPServer socket listen error!\n");
        gatIofSocketClose(fd);
        return GAT_ERR_FAIL;
    }
    printf("TCPServer create ok \n");

    return fd;
}

int32  gatIofTcpServerListen(int32 fd)
{
    gat_fd_set rfds;
    struct sockaddr_in addr;
    int32 addrLen;
    int32 iRet;
    int32 cliFd;

    if(fd < 0)
    {
        return -1;
    }

    GAT_FD_ZERO(&rfds);
    GAT_FD_SET(fd, &rfds);
    iRet = gatIofSelect( fd + 1, &rfds, NULL, NULL,0,1000000 );
    if(iRet > 0)
    {
        if(GAT_FD_ISSET(fd, &rfds))
        {
            addrLen = sizeof(struct sockaddr_in);
            cliFd = accept(fd, (struct sockaddr*)&addr, &addrLen);
            return cliFd;
        }
    }
    return -1;
}

