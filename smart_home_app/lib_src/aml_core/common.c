/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "common.h"
#include <time.h>
#include <json-c/json.h>
#include "aml_gst_player_imp.h"
#include "aml_playermanager.h"


char time_str[21];
char * read_file(char *path,char *rambuff){
	FILE *fp = NULL;
	int n_read;

    if( !path || !rambuff ) return NULL;
    fp = fopen(path,"r");
    if( !fp ) return NULL;

    n_read = fread(rambuff,1,80,fp);
    fclose(fp);
    if(n_read > 0)
    {
    	return rambuff;
    }
	else
		return NULL;
}

char * get_time_string()
{
	time_t timer;
	struct tm * tblock = NULL;
	memset(time_str,0,21);
	timer = time(NULL);
    tblock = localtime(&timer);
	sprintf(time_str , "%d-%02d-%02d %02d:%02d:%02d",
				tblock->tm_year+1900,
				tblock->tm_mon+1,
				tblock->tm_mday,
				tblock->tm_hour,
				tblock->tm_min,
				tblock->tm_sec);

	return time_str;
}
int is_ssid_QLINK()
{
	FILE *fp;
	char * popen_cmd = "wpa_cli status |grep \"CMCC-QLINK\"";
	char tmp_buf[256]={0};

	gatPrintf( GAT_DEBUG,"%s\n",__func__);
	fp = popen(popen_cmd, "r");

	while(fgets(tmp_buf, 256, fp) != NULL)
	{
		gatPrintf( GAT_DEBUG,"fgets: %s\n",tmp_buf);
		if((strstr(tmp_buf, "CMCC-QLINK"))!=NULL)
		{
			pclose(fp);
			return 1;
		}
		memset(tmp_buf,0,256);
	}
	pclose(fp);

	return 0;
}

/************************************************************
** FunctionName : playermanager_play_text
** Description  : 播放文字
** Input Param  : str指向需要播放的文字
** Output Param : 
** Return Value : 
**************************************************************/
int playermanager_play_text(char* str)
{
	playermanager_list_add(em_pm_playertype_tts, str);
	playermanager_play(em_pm_playertype_tts);

	return 0;
}

/************************************************************
** FunctionName : playermanager_play_file
** Description  : 播放指向路径的文件
** Input Param  : file_path，需要播放的文件路径
** Output Param : 
** Return Value : 
**************************************************************/
int playermanager_play_file(const char* file_path)
{
	playermanager_list_add(em_pm_playertype_notice, file_path);
	playermanager_play(em_pm_playertype_notice);
	
	return 0;
}

