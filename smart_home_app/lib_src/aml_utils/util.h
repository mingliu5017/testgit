/***************************************************************************
** CopyRight: Amlogic             
** Author   : jian.cai@amlogic.com
** Date     : 2018-08-09
** Description 
**  
***************************************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_SIZE(x) 			(sizeof(x)/sizeof(x[0]))

#define BIT_SET(data, pos)	   (data |= (1 << pos))
#define BIT_CLR(data, pos)     (data &= ~(1 << pos))
#define BIT_VALUE(data, pos)   (((data) >> pos) & 0x1)

typedef enum
{
	em_systime_boot = 0, /* 从开机启动开始计算的时间，不受用户设置时间的影响 */
	em_systime_utc, /* UTC时间，如果系统时间更新，则会受到影响 */
}em_sys_time;
 

/************************************************************
** FunctionName : printhex
** Description  : 16进制打印
** Input Param  :
** Output Param :
** Return Value :
**************************************************************/
void printhex(const unsigned char *pBuf, int bufSize);

/************************************************************
** FunctionName : getcurtime_str
** Description  : 获取时间字符串 例如"[2018-08-03 09:00:00.123]"
** Input Param  : 
** Output Param : 
** Return Value : 
**************************************************************/
const char *getcurtime_str();

/************************************************************
** FunctionName : getcurtime_ms
** Description  : 获取当前时间
** Input Param  : em_sys_time type
** Output Param : 
** Return Value : 
**************************************************************/
unsigned long getcurtime_ms(em_sys_time type);

#ifdef __cplusplus
}
#endif
#endif


