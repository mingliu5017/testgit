/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "iofpthread.h"

int32  iofPthreadCreate( gatpthread_t *thread,gatpthread_attr_t *attr,void *(*start_routine) (void *) , void *arg)
{
    return pthread_create( thread, attr,start_routine,arg );
}
int32  iofPthreadNameSet( int8 *szPthreadName )
{
    return prctl(PR_SET_NAME, szPthreadName );
}