/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef GATPLATFORMTYPE_H_
#define GATPLATFORMTYPE_H_
#include "gatplatform.h"
#include "iofstring.h"

typedef fd_set              gat_fd_set;
typedef struct sockaddr     gatSockAddr;
typedef pthread_t           gatpthread_t;
typedef pthread_attr_t      gatpthread_attr_t;

#define GAT_FD_ZERO  FD_ZERO
#define GAT_FD_SET   FD_SET
#define GAT_FD_ISSET FD_ISSET
#endif