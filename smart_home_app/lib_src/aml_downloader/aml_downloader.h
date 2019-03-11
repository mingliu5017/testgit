/***************************************************************************
** CopyRight: Amlogic             
** Author   : jian.cai@amlogic.com
** Date     : 2018-09-13
** Description 
**  
***************************************************************************/
#ifndef _AML_DOWNLOADER_H
#define _AML_DOWNLOADER_H

#ifdef __cplusplus
extern "C"{ 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef enum {
	em_downloader_event_finish = 0,
	em_downloader_event_error,
	em_downloader_event_abort,
	em_downloader_event_progress,  //progress is (double*)param
}em_downloader_event;


/************************************************************
** FunctionName : aml_downloader_cb
** Description  : 
** Input Param  : 
┌───────────────────────────────────┬──────────────────────────────┐
│    em_downloader_event            │    param                     │
├───────────────────────────────────┼──────────────────────────────┤
│  em_downloader_event_finish       │    NULL                      │
├───────────────────────────────────┼──────────────────────────────┤
│  em_downloader_event_error        │    reason: const char *      │
├───────────────────────────────────┼──────────────────────────────┤
│  em_downloader_event_abort        │    reason: const char *      │
├───────────────────────────────────┼──────────────────────────────┤
│  em_downloader_event_progress     │    precent: (double*)param   │
└───────────────────────────────────┴──────────────────────────────┘
** Output Param : 
** Return Value : 
**************************************************************/
typedef void (*aml_downloader_cb)(em_downloader_event e, void *param);

typedef struct _st_aml_dlinfo st_aml_dlinfo;

/************************************************************
** FunctionName : aml_downloader_init
** Description  : 初始化下载
** Input Param  : 
** Output Param : 
** Return Value : st_aml_dlinfo *
**************************************************************/
st_aml_dlinfo *aml_downloader_init();


/************************************************************
** FunctionName : aml_downloader_init
** Description  : 初始化下载
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void aml_downloader_uninit(st_aml_dlinfo *pst_dlinfo);


/************************************************************
** FunctionName : aml_downloader_setcallback
** Description  : 设置下载事件回调
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
void aml_downloader_setcallback(st_aml_dlinfo *pst_dlinfo, aml_downloader_cb cb);

/************************************************************
** FunctionName : aml_downloader_seturl
** Description  : 设置下载url
** Input Param  : 
					res_url: url  
					userpassword: ftp用户名和密码，格式为   username:password  (用:隔开)
** Output Param :
** Return Value : true: success,  false: failed
**************************************************************/
bool aml_downloader_seturl(st_aml_dlinfo *pst_dlinfo, const char *res_url, const char *userpassword);

/************************************************************
** FunctionName : aml_downloader_setsavepath
** Description  : 设置下载文件存放绝对路径。需保证目录已经创建。
** Input Param  : 
** Output Param : 
** Return Value : true: success,  false: failed
**************************************************************/
bool aml_downloader_setsavepath(st_aml_dlinfo *pst_dlinfo, const char *filefullpath);

/************************************************************
** FunctionName : aml_downloader_start
** Description  : 启动下载线程
** Input Param  : 
** Output Param : 
** Return Value : true: success,  false: failed
**************************************************************/
bool aml_downloader_start(st_aml_dlinfo *pst_dlinfo);


/************************************************************
** FunctionName : aml_downloader_abort
** Description  : 放弃下载
** Input Param  : 
** Output Param : 
** Return Value : true: success,  false: failed
**************************************************************/
bool aml_downloader_abort(st_aml_dlinfo *pst_dlinfo);



#ifdef __cplusplus
}
#endif
#endif


