/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef IOF_PTHREAD_H_
#define IOF_PTHREAD_H_

#ifdef __cplusplus
extern "C"{ 
#endif

#include "gatplatformType.h"
#include "gattypes.h"
int32 iofPthreadCreate( gatpthread_t *thread,gatpthread_attr_t *attr,void *(*start_routine) (void *) , void *arg);
int32 iofPthreadNameSet( int8 *szPthreadName );



#ifdef __cplusplus
}
#endif

#endif
