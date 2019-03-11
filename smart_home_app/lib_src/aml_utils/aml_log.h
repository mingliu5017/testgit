/***************************************************************************
** CopyRight: Amlogic             
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-09
** Description 
**  
***************************************************************************/

#ifndef _AMLLOG_H_
#define _AMLLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LEVEL_INFO 		1
#define LEVEL_WARN		2
#define LEVEL_ERROR		3
#define LEVEL_FATAL		4
#define LEVEL_MAX		0xF

#define LOG_LEVEL_INFO(fmt, args...)   printf("%s[I] %s [%s, #%d] " fmt "\n", getcurtime_str(), LOG_TAG, __func__, __LINE__, ##args)
#define LOG_LEVEL_WARN(fmt, args...)   printf("%s[W] %s [%s, #%d] " fmt "\n", getcurtime_str(), LOG_TAG, __func__, __LINE__, ##args)
#define LOG_LEVEL_ERROR(fmt, args...)  printf("%s[E] %s [%s, #%d] " fmt "\n", getcurtime_str(), LOG_TAG, __func__, __LINE__, ##args)
#define LOG_LEVEL_FATAL(fmt, args...)  printf("%s[F] %s [%s, #%d] " fmt "\n", getcurtime_str(), LOG_TAG, __func__, __LINE__, ##args)

#define PRINT_LEVEL 	LEVEL_INFO


#define LOG(level, fmt, args...)  \
{ \
	if(level >= PRINT_LEVEL) { \
		LOG_##level(fmt, ##args);\
	} \
}

#define INFO(fmt, args...) LOG(LEVEL_INFO, fmt,##args)
#define WARN(fmt, args...) LOG(LEVEL_WARN, fmt,##args)
#define ERROR(fmt, args...) LOG(LEVEL_ERROR, fmt,##args)
#define FATAL(fmt, args...) LOG(LEVEL_FATAL, fmt,##args)

#define LOGI(fmt, args...) LOG(LEVEL_INFO, fmt,##args)
#define LOGW(fmt, args...) LOG(LEVEL_WARN, fmt,##args)
#define LOGE(fmt, args...) LOG(LEVEL_ERROR, fmt,##args)
#define LOGF(fmt, args...) LOG(LEVEL_FATAL, fmt,##args)

extern const char *getcurtime_str();

#ifdef __cplusplus
}
#endif
#endif


