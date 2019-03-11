
#ifndef __SAI_SDK_INTERFACE_H__
#define __SAI_SDK_INTERFACE_H__
#include <stdbool.h>
#if defined(__cplusplus)
	extern "C" {
#endif

extern void start_sdk_service(const char * cfg_path);
extern void stop_sdk_service(void);
//用于用户id设置，一般为NULL，如果需要设置，在start_sdk_service 之前设置,id len <128
extern void set_user_id(const char * id);
//用于获取同步服务的token 如果需要，在start_sdk_service之前设置
extern void register_get_token_cb(void(*get_token_cb)(const char *token));
/*
* the value unit is millisecond
*/
extern void set_vad_timeout(time_t value);



typedef enum {
	KEY_EVT_VOL_UP = 1,
	KEY_EVT_VOL_DOWN,
	KEY_EVT_MUTE,
	KEY_EVT_PAUSE_AND_RESUME,
	KEY_EVT_TURN_ON, // system turn on , 
	KEY_EVT_NEXT,    // NEXT MUSIC
	KEY_EVT_LAST,    // LAST MUSIC
	KEY_EVT_VOIP_START,
	KEY_EVT_VOIP_END,
	KEY_EVT_NET,     // connect net 
	KEY_EVT_TEST,
	KEY_EVT_FM,
	KEY_EVT_LED,    //保留
}eKEY_EVENT;

extern void send_key_evt(eKEY_EVENT key_evt);
typedef void (*fLED_ARGS_CB)(unsigned int args);
typedef void (*fLED_NO_ARGS_CB)(void);
typedef struct {
        fLED_ARGS_CB          wk_cb;
        fLED_ARGS_CB      vadend_cb;
        fLED_NO_ARGS_CB    tts_play_cb;  //tts开始进行播放
        fLED_NO_ARGS_CB  mute_start_cb;  // mic 关闭
		fLED_NO_ARGS_CB	mute_stop_cb;    // mic 启用
        fLED_NO_ARGS_CB   start_sys_cb;  //sdk 启动
        fLED_ARGS_CB  dialog_end_cb;     // 对话结束
        fLED_ARGS_CB     vol_val_cb;
        fLED_NO_ARGS_CB     cfg_net_cb;  //播放配网声音
        fLED_NO_ARGS_CB     net_dis_cb;  //没有链接上服务器
        fLED_NO_ARGS_CB  start_play_cb; // 开始播放音乐
        fLED_NO_ARGS_CB   stop_play_cb;  // 结束播放
        fLED_NO_ARGS_CB        test_cb;
        fLED_NO_ARGS_CB  start_voip_cb;  //开启voip 
        fLED_NO_ARGS_CB   stop_voip_cb;
		fLED_NO_ARGS_CB net_connected_cb;
		fLED_NO_ARGS_CB tts_end_cb;      //tts 播放结束
} fLED_EVT_CB;

extern int register_led_cb(fLED_EVT_CB* user_led_cb);



extern const char *  get_sdk_version(void);
// tSAI_MUSIC 表示用户待播放的一首歌曲信息
struct tSAI_MUSIC{
	char * tts_content; // 播放的tts文本，不要超过200byte
	char * music_url;    // 歌曲 url;
}; 

// music callback
// inpput: 
// 1.semantics_msg 为sai nlp返回的json字符串，包含了用户输入命令和相关歌曲信息，使用者可以提取关注
// 的信息处理使用
// 2.int *user_music_nr 用户待播放歌曲的数目，成功时返回实际待播放歌曲的数据，否则-1；
//output:
//struct tSAI_MUSIC  *  用户需要播放的歌曲信息，可以传入歌单数组，用户分配内存，管理内存
typedef  struct tSAI_MUSIC *(*fSEMANTICS_MUSIC_CB)(const char *semantics_msg,int * user_music_nr);
extern void register_semantics_music_cb(fSEMANTICS_MUSIC_CB  music_cb);

//input :
// 1.semantics_msg 为sai nlp返回的json字符串，包含了用户输入命令和smarthome相关信息，使用者可以提取关注
// 2. tts_content 待播放的tts文本信息，用户不用管理内存，最大长度1K
// 3.tts_flg 为1时，需要播放tts，or 0
//output:
typedef void(*fSEMANTICS_SMARTHOME_CB)(const char *semantics_msg,char *tts_content,int*tts_flag);
extern void register_smarthome_cb(fSEMANTICS_SMARTHOME_CB smarthome_cb);

typedef	void(*fCHAT_CB)(const char *semantics_msg,char * tts_content,int *tts_flag);
extern void register_chat_cb(fCHAT_CB chat_cb);

typedef	void(*fNONETWORK_CB)(int *tts_flag);
extern void register_nonetwork_cb(fNONETWORK_CB nonetwork_cb);

//set volume val 
//input:
//1.value 0~100
//output
// ok 1 ,other failed;
typedef int(*fSET_VOLUME_VAL)(int value);
//get volume val
//input:  
//output:
// volume value 0~100,other failed
typedef int(*fGET_VOLUME_VAL)(void);
extern void register_set_volume_cb(fSET_VOLUME_VAL set_volume);
extern void register_get_volume_cb(fGET_VOLUME_VAL get_volume);

//phone_callback
//
//input:
// semantics_msg, 包含用户打电话意图的json
//tts_conten,用户待播放的tts内容,最大支持1024字节长度，用户无需分配,需要播放时设置tts_flag 为1，否则为0
//output:
//如果需要开启voip，返回1,否则返回0，
typedef int(*fPHONE_CB)(const char*semantics_msg,char* tts_content,int * tts_flag);
//phone_voip_data callback
//intput:
//voip_buffer,voip 数据，16k 单通道，长度为buffer_szie
// stop_flag ,标示是否结束voip功能，stop_flag 为1时，用户需要结束voip功能
typedef void(*fPHONE_VOIP_DATA_CB)(const char * voip_buffer,size_t buffer_size,int stop_flag);
extern void register_phone_cb(fPHONE_CB phone_cb);
extern void register_voip_data_cb(fPHONE_VOIP_DATA_CB voip_data_cb);

/*
notify network configuration status
	0: normal status
	1: in network configuration stage
*/
extern void net_cfg_status_notification(int netstatus);

// 任何SoundAI 无法识别的领域，都会调用该回调，供用户自己解析使用
// custom cb 返回 值 1，再次手动唤醒，0不唤醒
extern void register_customization_cb(int(*custom_cb)(const char *nlp_json,char*tts_content,int*tts_flg));

extern void register_health_cb(int(*health_cb)(const char * semantics_msg,char * tts_content,int * tts_flg));

extern void sai_set_volume_step(unsigned int step);

typedef enum {
    TTS_TYPE = 1,
    MUSIC_TYPE = 2,
    TTS_MUSIC_TYPE = 3,
}eVLC_PLAYER_TYPE;

extern void  sai_media_resume(eVLC_PLAYER_TYPE type);
extern void  sai_media_pause(eVLC_PLAYER_TYPE type);
extern void  sai_media_stop(eVLC_PLAYER_TYPE type);

// 供用户播放tts声音，data必须只能是文本，不能是链接 ,状态使用led tts 回调
extern void sai_media_play_user_tts(const char * data);


extern void sai_media_play_user_audio(const char * audio);

extern void sai_media_clear_music();
extern int sai_media_get_music_number();
extern int  sai_media_add_tts_and_music(const char * tts,const char * uri);
//更新music uri, index (0 - get_music_number-1)
extern int  sai_media_update_music_uri_ext(int index,const char * uri);
extern void sai_media_audio_play();

//按照用户设置的index播放歌曲，index 在0～ get_music_number-1
extern int sai_media_audio_set_index(int index);
//获取当前播放歌曲的index，正确的值：0～ get_music_number-1, -1 没有播放
extern int sai_media_audio_get_index();
//设置联系人信息
extern void sai_set_contact_info(const char *json);
//调整联系人匹配门限，percent 输入值：0-100
extern void sai_set_contact_match_threshold(int percent);


typedef enum {
    PREVIOUS,
    NEXT,
	SINGLE,
	LOOP,
	RANDOM,
	NORMAL,
}eMUSIC_MODE;

typedef enum {
	RESOURCE_NONE,
	RESOURCE_EXTERNAL,
	RESOURCE_INTERNAL,
}eRESOURCE_TYPE;

extern void sai_audio_play_pre_next(eMUSIC_MODE e);
extern void sai_audio_set_third_resource(bool isThirdResource);//this function will be replaced with sai_audio_set_resource_type
extern void sai_audio_set_resource_type(eRESOURCE_TYPE eResource);
//if 1 the audio play will stop when mute/unmute, the default value: mute:1 unmute:0
extern void sai_mute_stop_audio_play(int stopaudioplay);
extern void sai_unmute_stop_audio_play(int stopaudioplay);

typedef enum {
	TTS_EVT=1,
	AUDIO_EVT,
}eAUDIO_TTS_EVT;

typedef enum {
	AUDIO_PLAY = 1,
	AUDIO_PAUSE,
	AUDIO_STOPED,
	AUDIO_END,
	AUDIO_ERROR,
}eAUDIO_EVT;
typedef enum {
	AUDIO_COMMAND_PREVIOUS,
	AUDIO_COMMAND_NEXT,
	AUDIO_COMMAND_PLAY,
	AUDIO_COMMAND_PAUSE,
	AUDIO_COMMAND_KEY_PLAY_PAUSE,
}eAUDIO_COMMAND;
	
void register_audio_command(void(*audio_command_cb)(eAUDIO_COMMAND command));

void register_audio_evt(void(*audio_evt_cb)(int index, eAUDIO_EVT evt1, eAUDIO_TTS_EVT evt2, eRESOURCE_TYPE eResource));

typedef enum {
	TTS_PLAY = 1,
	TTS_STOPED,
	TTS_END,
        TTS_ERROR,
}eTTS_EVT;

void register_tts_evt(void(*tts_evt_cb)(eTTS_EVT evt));

/*
* send message to app, the max length is 2048
*/
extern void send_msg_to_app(const char * msg);

//wake up the system right now
extern void ex_set_wakeup_status();
//set beamforming angle
//@param value: angle value
extern void ex_set_talking_angle(float value);

#if defined(__cplusplus)
}
#endif
#endif
