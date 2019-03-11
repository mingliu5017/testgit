/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include "gattypes.h"
#include "gatlist.h"
#include <unistd.h>
#include<sys/wait.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DATA_FILE  "/data/data.json"
#define TEL_FILE  "/data/telphone.json"

char * read_file(char *path,char *rambuff);
/************************************************************
** FunctionName : playermanager_play_text
** Description  : 播放文字
** Input Param  : str需要播放的文字
** Output Param : 
** Return Value : 
**************************************************************/
int playermanager_play_text(char* str);

/************************************************************
** FunctionName : playermanager_play_file
** Description  : 播放指向路径的文件
** Input Param  : file_path，需要播放的文件路径
** Output Param : 
** Return Value : 
**************************************************************/
int playermanager_play_file(const char* file_path);

#ifdef __cplusplus
}
#endif

#endif
