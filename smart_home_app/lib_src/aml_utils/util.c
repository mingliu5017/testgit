/***************************************************************************
** CopyRight: Amlogic
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-09
** Description
**
***************************************************************************/

#define LOG_TAG "util"

#include "aml_log.h"
#include "util.h"
#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>


const char HexChar[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

/************************************************************
** FunctionName : printhex
** Description  : 16进制打印
** Input Param  :
** Output Param :
** Return Value :
**************************************************************/
void printhex(const unsigned char *pBuf, int bufSize)
{
	const int MAX_BUF_SIZE = 256;
	int i = 0, j = 0;
	int maxLen = bufSize > MAX_BUF_SIZE ? MAX_BUF_SIZE : bufSize;
	char outStr[MAX_BUF_SIZE * 4];

	memset(outStr, 0, sizeof(outStr));

	for(i = 0; i < maxLen; ) {
		outStr[j++] = HexChar[(pBuf[i] & 0xF0) >> 4];
		outStr[j++] = HexChar[pBuf[i] & 0x0F];
		outStr[j++] = ' ';
		i++;
		
		if(i % 16 == 0) {
			outStr[j++] = '\n';
		}
	}

	outStr[j++] = 0;

	LOG(LEVEL_INFO, "%s", outStr);
}


const char *getcurtime_str()
{
	static char timestring[128] = {0};

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

unsigned long getcurtime_ms(em_sys_time type)
{
	if( type == em_systime_boot ) {
		struct timespec t;
		t.tv_sec = t.tv_nsec = 0;
		clock_gettime(CLOCK_MONOTONIC, &t);
		return ((t.tv_sec * 1000) + (t.tv_nsec / (1000 * 1000)));
	} else { //if( type == em_systime_utc)
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
	}
}



