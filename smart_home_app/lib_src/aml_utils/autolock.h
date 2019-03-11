#ifndef _AUTOLOCK_H
#define _AUTOLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>


#define ENABLE_LOG  0
#define AUTOLOCK_TXT_LEN  256


class AutoLock
{
public:
	AutoLock(pthread_mutex_t *mutex, const char *str, int line)
	{
		pmLock = mutex;
		pthread_mutex_lock( pmLock );

#if ENABLE_LOG
		memset(mFuncName, 0, AUTOLOCK_TXT_LEN);
		strncat(mFuncName, str, AUTOLOCK_TXT_LEN - 1);
		mLine = line;
		printf("%s [AutoLock] %s %d lock....\n", GetCurTimeStr(), mFuncName, mLine);
#endif
	}


	~AutoLock()
	{
#if ENABLE_LOG
		printf("%s [AutoLock] %s %d unlock....\n", GetCurTimeStr(), mFuncName, mLine);
#endif

		pthread_mutex_unlock( pmLock );
	}

private :
	const char *GetCurTimeStr()
	{
		static char timestring[64] = {0};

		time_t timep;
		struct  timeb stTimeb;
		struct tm *p;

		time(&timep);
		p = localtime(&timep);
		ftime(&stTimeb);

		memset(timestring, 0, sizeof(timestring));
		sprintf(timestring, "[%d-%02d-%02d %02d:%02d:%02d.%03d] ", (1900 + p->tm_year), ( 1 + p->tm_mon), p->tm_mday,
		        p->tm_hour, p->tm_min, p->tm_sec, stTimeb.millitm);

		return (const char *)&timestring[0];
	}

private:
	pthread_mutex_t *pmLock;

#if ENABLE_LOG
	char mFuncName[AUTOLOCK_TXT_LEN];
	int mLine;
#endif
};

#endif




