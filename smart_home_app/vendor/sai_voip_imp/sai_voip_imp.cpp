#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sai_voip_imp.h"
#include "sound_ai/include/sai_sdk_interface.h"

//#define RECORD_TO_FILE

#ifdef RECORD_TO_FILE
static const char *g_pstr_file = "/tmp/voip_pcm.dat";
static FILE *g_pFile = NULL;
#endif


static VOIP_DATA_CB g_voip_cb = NULL;

static void voip_data_cb(const char *buffer,size_t size,int flag)
{
    if(flag){
        return;
    }else if(g_voip_cb != NULL){
		g_voip_cb(buffer, size);
		
#ifdef RECORD_TO_FILE
		if(g_pFile != NULL) {
			fwrite(buffer, size, 1, g_pFile);
		}
#endif
	}
}


void sai_voip_imp_init(const char *config_path)
{
	start_sdk_service(config_path);
	register_voip_data_cb(voip_data_cb);
}

void sai_voip_imp_setcallback(VOIP_DATA_CB callback)
{
	g_voip_cb = callback;
}

void sai_voip_imp_start()
{
	send_key_evt(KEY_EVT_VOIP_START);
	
#ifdef RECORD_TO_FILE
	if(g_pFile == NULL){
		g_pFile = fopen(g_pstr_file, "wb");
	}
#endif
}

void sai_voip_imp_stop()
{
	send_key_evt(KEY_EVT_VOIP_END);

#ifdef RECORD_TO_FILE
	if(g_pFile != NULL){
		usleep(200 * 1000);
		fflush(g_pFile);
		fclose(g_pFile);
		g_pFile = NULL;
	}
#endif	
}

