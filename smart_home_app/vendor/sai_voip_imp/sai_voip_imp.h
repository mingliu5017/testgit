#ifndef _sai_voip_imp_h_
#define _sai_voip_imp_h_

#ifdef __cplusplus
extern "C"{ 
#endif


typedef void (*VOIP_DATA_CB)(const char *buffer, size_t size);


void sai_voip_imp_init(const char *config_path);

void sai_voip_imp_setcallback(VOIP_DATA_CB callback);

void sai_voip_imp_start();

void sai_voip_imp_stop();


#ifdef __cplusplus
}
#endif
#endif


