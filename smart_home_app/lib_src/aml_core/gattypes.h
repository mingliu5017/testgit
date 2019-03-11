/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _GAGENTTYPES_H
#define _GAGENTTYPES_H
#include "log.h"
#include "gatplatformType.h"
#ifndef NULL
#define NULL 0
#endif

#define GAT_OK                  0
#define GAT_ERR_FAIL            (-1)    /**/
#define GAT_ERR_TIMEOUT         (-2)
#define GAT_ERR_NORES           (-3)    /* no resourse */
#define GAT_ERR_PARAM           (-4)    /* param err */

#define INVALID_SOCKET         (-1)

typedef signed char     int8;
typedef unsigned char   uint8;
typedef signed short    int16;
typedef unsigned short  uint16;
typedef signed int      int32;
typedef unsigned int    uint32;
typedef int             lock_t;

#define MAC_LEN     12
#define PASSCODE_LEN     11

#define IP_LEN_MAX      16
#define DOMAIN_LEN_MAX      128

typedef struct _gatTimer_t
{
   int32 (*timerCB)(struct _gatTimer_t *gatTimer );
   uint32 period;            
   uint8* szName; 
   void* addr;
   void* param;
}gatTimer_st;

#endif /* _GAGENTTYPES_H */

