/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _AGENT_TIMER_LIST_
#define _AGENT_TIMER_LIST_
#ifdef __cplusplus
extern "C"{ 
#endif




#include "gatlist.h"
#include "gattypes.h"
#include "iofipc.h"
#include "statusevent_manage.h"

#define SYS_MUTEXLOCK_SUPPORT
#define TIMER_HANDLE_MODE              TIMER_HANDLE_MULT
#ifdef  SYS_MUTEXLOCK_SUPPORT
#define TIMER_MUTEX_LOCK(x)      iofLock(&x)
#define TIMER_MUTEX_UNLOCK(x)    iofUnlock(&x)
#else
#define TIMER_MUTEX_LOCK(x)  ((x == TIMER_LOCK) ? -1 : (x = TIMER_LOCK))
#define TIMER_MUTEX_UNLOCK(x)  (x = TIMER_UNLOCK)
#endif

#define TIMER_UNLOCK                   0
#define TIMER_LOCK                     1
#define TIMER_HANDLE_SINGLE            0
#define TIMER_HANDLE_MULT              1

#define TIMER_EXPIRE_MAX               0xFFFFFFFF
#define TIMER_EXPOSE_MAX               0X7FFFFFFF

typedef struct _agent_Timer_Core
{
    struct gatListHead timerList;
    uint32 count;               
    uint32 expose;              
    lock_t lock;
}agentTimerCore_st, *pAgentTimerCore_st;

typedef struct _agent_timer_st
{
   struct gatListHead entry;
   gatTimer_st *appTimer;        

   // int32 (*timerCB)(struct _agent_timer_st *timer); 
   uint32 expire;              
   // uint32 period;              
   uint8 _linked;              
   // char* szName;               
   // void* param;                
}gatHalTimer_st;

typedef int32 (*timerCB)(gatTimer_st *pTimer);

void  gatTimerCoreInit();

int32  gatTimerAdd(gatTimer_st *pTimer,
                        char* szName,
                        uint32 expire,
                        timerCB functionCB,
                        void* param);

int32  gatTimerDel(gatTimer_st* pTimer);
int32  gatTimerTraversal(void);
int32 gatTimerHandle(gatTimer_st** appTimer);

#ifdef __cplusplus
}
#endif

#endif
