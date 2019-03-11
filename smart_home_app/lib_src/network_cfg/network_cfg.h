#ifndef __NET_CFG_H
#define __NET_CFG_H

#include <stdio.h>
#include <stdlib.h>

/************************************************************
** FunctionName : get_ip_addr
** Description  : 获取当前IP地址
** Input Param  : 无
** Output Param : IP地址
** Return Value : 成功：IP地址 失败：NULL
**************************************************************/
char * get_ip_addr(char *ip_address);

/************************************************************
** FunctionName : check_wifi_status
** Description  : wifi是否连接成功
** Input Param  : 无
** Output Param : 无
** Return Value : 成功：1 失败：0
**************************************************************/
int check_wifi_status();

/************************************************************
** FunctionName : set_wifi_config
** Description  : 设置wifi ssid 与psk 并生效
** Input Param  : SSID passwd
** Output Param : 无
** Return Value : 0：成功 -1 失败
**************************************************************/
int set_wifi_config(const char * SSID,const char * passwd);

/************************************************************
** FunctionName : start_config_wifi_with_BT
** Description  : 启动蓝牙配网
** Input Param  : 无
** Output Param : 无
** Return Value : 无
**************************************************************/
void start_config_wifi_with_BT(void);

/************************************************************
** FunctionName : get_BT_Mac_addr
** Description  : 获取蓝牙MAC地址
** Input Param  : 无
** Output Param : IP地址
** Return Value : 成功：IP地址 失败：NULL
**************************************************************/
char* get_BT_Mac_addr(void);

/************************************************************
** FunctionName : get_bcast_addr
** Description  : 获取获取网关地址
** Input Param  : 无
** Output Param : bcast_address 网关地址
** Return Value : 成功：网关地址 失败：NULL
**************************************************************/
char * get_bcast_addr(char *bcast_address);

#endif
