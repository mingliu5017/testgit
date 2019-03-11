#include "ledring.h"

#define CMD_LEDRING_SETUP		0x100001
#define CMD_LEDRING_VERSION		0x100002
#define CMD_LEDRING_VOLUME		0x100003
#define CMD_LEDRING_WAKEUP		0x100004
#define CMD_LEDRING_SELFDEF		0x100005
#define CMD_LEDRING_SELFSIG     0x100006

typedef enum
{
	LED_SIG_ROLL = 0x01,     /*led 自定义无底色单色旋转*/
	LED_ROLL_BAK,            /*led 有底色单色旋转*/
	LED_ALL_FLASH,           /*自定义RGB闪烁，所有灯相同*/
	LED_ALL_ROLL,            /*自定义每颗led，所有led旋转*/
	LED_SIG_FLASH,           /*自定义每颗led闪烁，led颜色不同*/
	LED_ROLL_POSITION,      /*有背景色，带有方向旋转*/
	LED_ALL_LIGHT = 0x0f,     /*自定义led常亮，所有led颜色相同*/
	LED_SIG_LIGHT            /*自定义led常亮，led颜色不同*/
} LED_PATTERN;

typedef struct {
	int period;
	LED_PATTERN pattern;
    unsigned char duration_time;
	unsigned char position;
	unsigned char led_all_data[3];
	unsigned char led_sig_data[24];
}led_info;

int _ledfd;

/* R   G   B */
int ledcolors[8][3]={
		   {130,  5, 40},           /*咪咕粉*/
		   {  0,  0,250},			/*蓝*/
		   {200,  0,  0},           /*警示红*/
		   {255,133,0  },           /*橙*/
		   {237,147,199},           /*粉白*/
		   {255,178,103},           /*橙白*/
		   {255,255,255},           /*白*/
		   {26,   0,  0}};          /*红1*/
int led_blue_table[10][3]={
		   {  0,  0, 50},           /*咪咕粉*/
		   {  0,  0,100},			/*蓝*/
		   {  0,  0,150},           /*警示红*/
		   {  0,  0,200},           /*警示红*/
		   {  0,  0,255},           /*警示红*/
		   { 26, 26,255},           /*警示红*/
		   { 51, 51,255},           /*警示红*/
		   { 77, 77,255},           /*警示红*/
		   {102,102,255},           /*警示红*/
		   {128,128,255}};          /*警示红*/

/*    粉     |    橙    |     粉     |    橙   */
/* G   R   B   G   R   B   G   R   B   G   R   B*/
int ledsigle[5][24]={
			 {  5,130, 40,120,240, 0 ,  5,130, 40,120,240, 0 ,\
		        5,130, 40,120,240, 0 ,  5,130, 40,120,240, 0},  /*咪咕粉橙*/

		     {147,237,199,178,255,103,147,237,199,178,255,103,\
		      147,237,199,173,255,103,147,237,199,178,255,103},  /*粉白橙白*/

		     {101,238,173,255,255,255,101,238,173,255,255,255,\
		      101,238,173,255,255,255,101,238,173,255,255,255},   /*咪咕粉白*/

		     {187, 0 ,250,255,255,255,187, 0 ,250,255,255,255,\
		      187, 0 ,250,255,255,255,187, 0 ,250,255,255,255},    /*咪咕蓝白*/

		     {101,238,173,101,233,173,101,233,173,101,233,173,\
		      101,238,173,101,233,173,101,233,173,101,233,173}}; /*咪咕粉*/


int ledControl(int cmd,int durationTime, int args)
{
	int sbuf[30],i,j;
	int value,m,n,flag;
	led_info ledctl;

	memset(&ledctl,0,sizeof(led_info));
	switch (cmd)
	{
	  case WAKE_UP: 
		  for(i=0;i<24;i++){
			  ledctl.led_sig_data[i] = ledsigle[0][i];
		  }
		  if (args > 8) ledctl.position =8;
		  else if (args < 0) ledctl.position =0;
		  else
		  ledctl.position = args;
		  ledctl.pattern = LED_ROLL_POSITION;
		  //转圈白色灯
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[6][i];
		  }
		  ledctl.period = 4;						 //转圈时间间隔，以10ms为单位，80即800msa
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFSIG;
		  printf("set led WAKE_UP\n");
		  break;
	  case MIGU_FEN_5:
		  //请输入亮度级别(1-10):);
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[0][i];
		  }
		  ledctl.pattern = LED_ALL_LIGHT;
		  flag = CMD_LEDRING_SELFDEF;

		  ledctl.duration_time = durationTime;
		  printf("set led MIGU_FEN_5\n");
		  break;
	  case BLUE_5:
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[1][i];
		  }
		  ledctl.pattern = LED_ALL_LIGHT;
		  flag = CMD_LEDRING_SELFDEF;
		  ledctl.duration_time = durationTime;
		  printf("set led BLUE_5\n");
		  break;

	  case RED_5:
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[2][i];
		  }
		  ledctl.pattern = LED_ALL_LIGHT;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;

		  printf("set led RED_5\n");
		  break;
	  case RED_1:
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[7][i];
		  }
		  ledctl.pattern = LED_ALL_LIGHT;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;
		  printf("set led RED_1\n");
		  break;
	  case WHITE_5:
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[6][i];
		  }
		  ledctl.pattern = LED_ALL_LIGHT;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;
		  printf("set led WHITE_5\n");
		  break;
	  case BLUE_VOL:
		  //请输入音量(1-10):
		  value = args;
		  if (value < 0)
			  value = 1;
		  if (value > 10)
			  value = 10;
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = led_blue_table[value][i];
		  }
		  ledctl.pattern = LED_ALL_LIGHT;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;
		  printf("set led BLUE_VOL:[%d]\n",value );
		  break;
	  case ALL_OFF:
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = 0;
		  }
		  ledctl.pattern = LED_ALL_LIGHT;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;
		  printf("set led ALL_OFF\n");
		  break;
	  case WHITE_BLINK:
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[6][i];
		  }
		  ledctl.period = 80;
		  ledctl.pattern = LED_ALL_FLASH;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;
		  printf("set led WHITE_BLINK\n");
		  break;
	  case WHITE_ROLLING:
		  for(j=0;j<3;j++){
			  ledctl.led_all_data[j] = ledcolors[6][j];
		  }
		  ledctl.period = 5;
		  ledctl.pattern = LED_SIG_ROLL;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;
		  printf("set led WHITE_ROLLING\n");
		  break;
	  case MIGU_FEN_CHENG:
		  for(i=0;i<24;i++){
			  ledctl.led_sig_data[i] = ledsigle[0][i];
		  }
		  ledctl.pattern = LED_SIG_LIGHT;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFSIG;
		  printf("set led MIGU_FEN_CHENG\n");
		  break;
	  case BLUE_BLINK:
		  for(i=0;i<3;i++){
			  ledctl.led_all_data[i] = ledcolors[1][i];
		  }
		  ledctl.period = 80;
		  ledctl.pattern = LED_ALL_FLASH;
		  ledctl.duration_time = durationTime;
		  flag = CMD_LEDRING_SELFDEF;
		  printf("set led BLUE_BLINK\n");
		  break;
	  default:
		  break;
	}
	ioctl(_ledfd, flag, &ledctl);

  return 0;
}

int led_all_off_mute(void)
{
	ledControl(ALL_OFF,0,0);
}
int red5_to_red1TimerCb()
{
	return ledControl(RED_1,0,0);
}

int ledShowVolume(volumeLedStyle _ledStyle) {
	return ledControl(BLUE_VOL,0,_ledStyle);
}

int ledInit(void) {
	_ledfd = open("/dev/ledring", O_RDWR);
	return 0;
}

void led_deInit(void) {
    (void)close(_ledfd);
	return;
}

