/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "ioftime.h"
#include <sys/time.h>

char *iofCurTime();
static void timerCb( int sig )
{
    static int timernum=0;
    if(SIGALRM == sig)
    {
        // printf("time=%d\n", ++timernum );
        gatTimerTick( 1 );
        alarm( 1 );             //we contimue set the timer
        //gatTimerTraversal();
    }
    return;
    printf("%s %d \n",__FUNCTION__,__LINE__ );
}

void iofTimerInit( void )
{
    signal( SIGALRM, timerCb ); //relate the signal and function
    alarm( 1 );                 //trigger the timer
    printf( "Time:%s\n",iofCurTime() );
}

char wt[32];
char *iofCurTime()
{
    struct tm *p;
    time_t t;
    struct timeval tt;
    int len;

    gettimeofday(&tt, NULL);
    time(&t);
    p = gmtime(&t);
    wt[0] = 0;
    len = sprintf(wt, "[%02d-%02d %02d:%02d:%02d.%06d]", p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec, (int)tt.tv_usec);
    if(len > 0)
    {
        wt[len] = 0;
    }
    return wt;
}
