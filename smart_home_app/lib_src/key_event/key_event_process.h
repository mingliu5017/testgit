#ifndef EVENTS_PROCESS_H
#define EVENTS_PROCESS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/input.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

/************************************************************
** FunctionName : key_eventprocess_init
** Description  : 按键事件处理线程初始化
** Input Param  : 无
** Output Param : 无
** Return Value : -1 ：失败  0：成功  配置文件路径：/etc/gpio_key.kl
**************************************************************/
int key_eventprocess_init(void);

/************************************************************
** FunctionName : WaitKey
** Description  : 获取按键事件
** Input Param  : 无
** Output Param : flag 1：长按  0：短按
** Return Value : 事件描述：  longpress 键值 或  键值
**************************************************************/
char* WaitKey(int* flag);

#ifdef __cplusplus
}
#endif

#endif  // EVENTS_PROCESS_H