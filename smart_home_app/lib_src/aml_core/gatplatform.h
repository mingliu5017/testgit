/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef GAT_PLATFORM_H_
#define GAT_PLATFORM_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <netdb.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <sys/prctl.h>
#include <sys/msg.h>

#define sockaddr_t sockaddr_in

/********time define***********/
#define ONE_SECOND                      (1)

#define GAT_PATH_NAME    "./"

#endif
