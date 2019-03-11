#ifndef __LEDRING_H
#define __LEDRING_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

typedef enum _ledCmd {
	WAKE_UP=0,       /* 粉色闪烁两下转一圈聚焦于一点，参数：灯转圈起始位置灯 */
	MIGU_FEN_5,      /*粉色常亮， 无参数*/
	BLUE_5,          /*蓝色常亮， 无参数*/
	RED_5,           /*警示红常亮， 无参数*/
	RED_1,           /*淡红常亮， 无参数*/
	WHITE_5,         /*白灯常亮， 无参数*/
	BLUE_VOL,        /*音量显示，参数：音量等级1-10*/
	ALL_OFF,         /*所有灯熄灭， 无参数*/
	WHITE_BLINK,     /*白色闪烁， 无参数*/
	WHITE_ROLLING,   /*白色旋转， 无参数*/
	MIGU_FEN_CHENG,  /*咪咕粉常亮， 无参数*/
	BLUE_BLINK,      /*蓝灯闪烁， 无参数*/
}ledCmd;

typedef enum __volumeLedStyle {
	VOLUME_LEVEL_MUTE = 0,		/* 静音 */
	VOLUME_LEVEL_0,				/* 音量等级0 */
	VOLUME_LEVEL_1,
	VOLUME_LEVEL_2,
	VOLUME_LEVEL_3,
	VOLUME_LEVEL_4,
	VOLUME_LEVEL_5,
	VOLUME_LEVEL_6,
	VOLUME_LEVEL_7,
	VOLUME_LEVEL_8,
	VOLUME_LEVEL_9,
	VOLUME_LEVEL_10,
	VOLUME_LEVEL_11,
	VOLUME_LEVEL_12,
	VOLUME_LEVEL_13,
	VOLUME_LEVEL_14,
	VOLUME_LEVEL_15,			/* 音量等级15*/
} volumeLedStyle;

/************************************************************
** FunctionName : ledInit
** Description  : LED驱动初始化，打开设备驱动节点
** Input Param  : 无
** Output Param : 无
** Return Value : -1 ：失败  0：成功
**************************************************************/
int ledInit(void);
/************************************************************
** FunctionName : led_deInit
** Description  : 释放led灯驱动
** Input Param  : 无
** Output Param : 无
** Return Value : 无
**************************************************************/
void led_deInit(void);
/************************************************************
** FunctionName : ledShowVolume
** Description  : LED显示音量灯效
** Input Param  : _ledStyle 音量等级
** Output Param : 无
** Return Value : 0：成功 -1 失败
**************************************************************/
int ledShowVolume(volumeLedStyle _ledStyle);
/************************************************************
** FunctionName : ledControl
** Description  : LED灯效选择控制
** Input Param  : cmd 灯显示类型  durationTime 延时时间  args 显示参数
** Output Param : 无
** Return Value : 0：成功 -1 失败
**************************************************************/
int ledControl(int cmd, int durationTime, int args);
/************************************************************
** FunctionName : red5_to_red1TimerCb
** Description  : red5灯常亮，用于定时关闭
** Input Param  : 无
** Output Param : 无
** Return Value : 0：成功 -1 失败
**************************************************************/
int red5_to_red1TimerCb(void);

/************************************************************
** FunctionName : led_all_off_mute
** Description  : 所有灯熄灭
** Input Param  : 无
** Output Param : 无
** Return Value : 0：成功 -1 失败
**************************************************************/
int led_all_off_mute(void);

#ifdef __cplusplus
}
#endif

#endif
