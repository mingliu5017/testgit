#ifndef _AML_UPDATE_H_
#define _AML_UPDATE_H_

#ifdef __cplusplus
extern "C"{ 
#endif


/************************************************************
** FunctionName : aml_update_setrecovery
** Description  : 启动recovery升级。
                  在升级文件下载完成之后调用，
                  调用之后会重启进入recovery开始升级
** Input Param  : 
** Output Param : 
** Return Value : 0:success other:failed
**************************************************************/
int aml_update_startupdate();


#ifdef __cplusplus
}
#endif
#endif


