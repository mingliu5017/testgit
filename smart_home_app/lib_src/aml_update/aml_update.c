#define LOG_TAG "aml_update"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "aml_log.h"
#include "aml_update.h"

extern int mtd_scan_partitions();
extern int set_recovery();

/************************************************************
** FunctionName : aml_update_setrecovery
** Description  : 启动recovery升级。
                  在升级文件下载完成之后调用，
                  调用之后会重启进入recovery开始升级
** Input Param  : 
** Output Param : 
** Return Value : 0:success other:failed
**************************************************************/
int aml_update_startupdate()
{
	mtd_scan_partitions();

	if(0 == set_recovery()){
		system("sync");
		system("reboot");
		return 0;
	}else{
		LOG(LEVEL_ERROR, "set_recovery failed!");
		return -1;
	}
}

