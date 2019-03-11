/***************************************************************************
** CopyRight: Amlogic
** Author   : jian.cai@amlogic.com
** Date     : 2018-10-09
** Description
**
***************************************************************************/
#define LOG_TAG "aml_playermanager"

#include <vector>
#include <string>

#include "aml_log.h"
#include "aml_playermanager.h"
#include "aml_tts_play.h"

#define  FILE_PATH_HEAD "file://"

using namespace std;

typedef unsigned char byte;


playermanager_event_callback event_callback = NULL;

static em_pm_playertype g_cur_player_type = em_pm_playertype_reserver;

//musicplayer
static vector<string> g_musiclist;
static int g_musiclist_playindex = 0;
static em_pm_playmode g_music_playmode = em_pm_playmode_order;
static st_gstplayer *g_music_pgstplayer = NULL;

//noticeplayer
static string g_notice_content;
static st_gstplayer *g_notice_pgstplayer = NULL;
static int notice_pcm_channels = 1;
static int notice_pcm_samplerate = 16000;
static snd_pcm_format_t notice_pcm_format = SND_PCM_FORMAT_S16_LE;

//ttsplayer
static aml_tts_interface *g_pttsplayer = NULL;
static string g_tts_content;

//textbookplayer
static vector<string> g_textbooklist;

static bool path_is_pcm(string path)
{
	int len = path.length();
	size_t found1 = g_notice_content.rfind(".pcm");
	size_t found2 = g_notice_content.rfind(".PCM");

	return (found1 == len - 4) || (found2 == len - 4);
}

static int get_format_bits(snd_pcm_format_t format)
{
	int bits = 8;
	switch(format) {
		case SND_PCM_FORMAT_S8:
		case SND_PCM_FORMAT_U8:
			bits = 8;
			break;
		case SND_PCM_FORMAT_S16_LE:
		case SND_PCM_FORMAT_S16_BE:
		case SND_PCM_FORMAT_U16_LE:
		case SND_PCM_FORMAT_U16_BE:
			bits = 16;
			break;
		case SND_PCM_FORMAT_S24_LE:
		case SND_PCM_FORMAT_S24_BE:
		case SND_PCM_FORMAT_U24_LE:
		case SND_PCM_FORMAT_U24_BE:
			bits = 24;
			break;
		case SND_PCM_FORMAT_S32_LE:
		case SND_PCM_FORMAT_S32_BE:
		case SND_PCM_FORMAT_U32_LE:
		case SND_PCM_FORMAT_U32_BE:
			bits = 32;
			break;
		default:
			bits = 8;
			break;
	}

	return bits;
}


static void create_wave_head(byte wavehead[], int pcm_length, int channels, int samplerate, int samplebits)
{
	const int headsize = 44;
	//RIFF WAVE Chunk // RIFF标记
	wavehead[0] = 'R';
	wavehead[1] = 'I';
	wavehead[2] = 'F';
	wavehead[3] = 'F';

	unsigned long totalDataLen = headsize - 8 + pcm_length;

	//数据大小
	wavehead[4] = (byte) (totalDataLen & 0xff);
	wavehead[5] = (byte) ((totalDataLen >> 8) & 0xff);
	wavehead[6] = (byte) ((totalDataLen >> 16) & 0xff);
	wavehead[7] = (byte) ((totalDataLen >> 24) & 0xff);

	//WAVE标记
	wavehead[8] = 'W';
	wavehead[9] = 'A';
	wavehead[10] = 'V';
	wavehead[11] = 'E';

	//FMT Chunk
	wavehead[12] = 'f'; // 'fmt '标记
	wavehead[13] = 'm';
	wavehead[14] = 't';
	wavehead[15] = ' ';//过渡字节

	//fmt数据大小
	wavehead[16] = 16; // 4 bytes: size of 'fmt ' chunk
	wavehead[17] = 0;
	wavehead[18] = 0;
	wavehead[19] = 0;

	//编码方式
	wavehead[20] = 1; // format = 1
	wavehead[21] = 0;

	//通道数
	wavehead[22] = (byte) channels;
	wavehead[23] = 0;

	//采样率
	wavehead[24] = (byte) (samplerate & 0xff);
	wavehead[25] = (byte) ((samplerate >> 8) & 0xff);
	wavehead[26] = (byte) ((samplerate >> 16) & 0xff);
	wavehead[27] = (byte) ((samplerate >> 24) & 0xff);

	//音频数据传送速率,采样率*通道数*采样深度/8
	int byteRate = samplerate * channels * samplebits / 8;
	wavehead[28] = (byte) (byteRate & 0xff);
	wavehead[29] = (byte) ((byteRate >> 8) & 0xff);
	wavehead[30] = (byte) ((byteRate >> 16) & 0xff);
	wavehead[31] = (byte) ((byteRate >> 24) & 0xff);

	//blocksize
	wavehead[32] = (byte) (channels * samplebits / 8);
	wavehead[33] = 0;

	//每个样本的数据位数
	wavehead[34] = samplebits;
	wavehead[35] = 0;

	//Data chunk
	wavehead[36] = 'd';//data标记符
	wavehead[37] = 'a';
	wavehead[38] = 't';
	wavehead[39] = 'a';

	//pcm数据长度
	wavehead[40] = (byte) (pcm_length & 0xff);
	wavehead[41] = (byte) ((pcm_length >> 8) & 0xff);
	wavehead[42] = (byte) ((pcm_length >> 16) & 0xff);
	wavehead[43] = (byte) ((pcm_length >> 24) & 0xff);
}


static const char *create_wav_from_pcm(const char *pcmfile, int channels, int samplerate, snd_pcm_format_t format)
{
#define READ_BUF_SIZE 2048

	FILE *file_pcm = fopen(pcmfile, "rb");
	if(file_pcm == NULL) {
		LOG(LEVEL_ERROR, "open %s failed!", pcmfile);
		return NULL;
	}

	fseek(file_pcm, 0L, SEEK_END);
	unsigned long filezize = ftell(file_pcm);
	if(filezize == 0) {
		LOG(LEVEL_ERROR, " %s file size is 0", pcmfile);
		fclose(file_pcm);
		return NULL;
	}

	const char *outputfile = "/tmp/_tmp_pcm2wav.wav";
	FILE *file_wav = fopen(outputfile, "wb");
	if(file_wav == NULL) {
		LOG(LEVEL_ERROR, "create %s failed!", outputfile);
		fclose(file_pcm);
		return NULL;
	}

	byte wavehead[44];
	create_wave_head(wavehead, filezize, channels, samplerate, get_format_bits(format));
	fwrite(&wavehead, sizeof(wavehead), 1, file_wav);

	byte buff[READ_BUF_SIZE];
	int read = 0;

	fseek(file_pcm, 0L, SEEK_SET);

	while((read = fread(buff, 1, READ_BUF_SIZE, file_pcm)) > 0) {
		fwrite(buff, 1, read, file_wav);
	}

	fflush(file_wav);
	fclose(file_wav);
	fclose(file_pcm);

	return outputfile;
}


void playermanager_init()
{

}

void playermanager_set_eventcallback(playermanager_event_callback cb)
{
	event_callback = cb;
}

void playermanager_uninit()
{
	if(g_music_pgstplayer != NULL) {
		aml_gstplayer_uninit(g_music_pgstplayer);
		g_music_pgstplayer = NULL;
	}

	if(g_notice_pgstplayer != NULL) {
		aml_gstplayer_uninit(g_notice_pgstplayer);
		g_notice_pgstplayer = NULL;
	}

	g_pttsplayer = NULL;
	g_musiclist_playindex = 0;
	g_musiclist.clear();
	g_textbooklist.clear();
}


void playermanager_set_ttsplayer(void *pttsplayer)
{
	if(pttsplayer != NULL) {
		g_pttsplayer = (aml_tts_interface *)pttsplayer;
	}
}


void playermanager_set_playmode(em_pm_playmode mode)
{
	g_music_playmode = mode;
}


void playermanager_list_add(em_pm_playertype type, const char *content)
{
	if(content == NULL) {
		return;
	}

	LOG(LEVEL_INFO, "content:%s", content);

	string path(content);

	if(em_pm_playertype_music == type) {
		if(path.at(0) == '/') {
			path.insert(0, FILE_PATH_HEAD);
		}
		g_musiclist.push_back(path);
	} else if(em_pm_playertype_notice == type) {
		if(path.at(0) == '/') {
			path.insert(0, FILE_PATH_HEAD);
		}
		g_notice_content = path;
	} else if(em_pm_playertype_tts == type) {
		g_tts_content = path;
	} else if(em_pm_playertype_txtbook == type) {
		g_textbooklist.push_back(path);
	}
}


void playermanager_list_clear(em_pm_playertype type)
{
	if(em_pm_playertype_music == type) {
		g_musiclist.clear();
		g_musiclist_playindex = 0;
	} else if(em_pm_playertype_txtbook == type) {
		g_textbooklist.clear();
	}
}


void playermanager_set_pcmparam(snd_pcm_format_t format, int samplerate, int channels)
{
	notice_pcm_channels = channels;
	notice_pcm_samplerate = samplerate;
	notice_pcm_format = format;
}

static void music_gstplayer_callback(st_gstplayer *pgstplayer, aml_gstplayer_event event, void *param)
{
	LOG(LEVEL_INFO, "event:%d param:%p", event, param);

	if(EM_GSTPLAYER_EVENT_PlayFinished == event || EM_GSTPLAYER_EVENT_PlayError) {
		int left = 0;
		const char *playurl_cstr = NULL;

		if(em_pm_playmode_order == g_music_playmode) {

			if(g_musiclist.size() > 0) {
				vector<string>::iterator k = g_musiclist.begin();
				g_musiclist.erase(k);
			}

			left = g_musiclist.size();
			if(left > 0) {
				string playurl = g_musiclist[g_musiclist_playindex];
				playurl_cstr = playurl.c_str();
			} else {
				if(event_callback != NULL) {
					event_callback(em_pm_playertype_music, em_pm_playerevent_allfinish);
				}
				return;
			}
		} else if(em_pm_playmode_single_cycle == g_music_playmode) {
			if(event_callback != NULL) {
				event_callback(em_pm_playertype_music, em_pm_playerevent_onefinish);
			}
			string playurl = g_musiclist[g_musiclist_playindex];
			playurl_cstr = playurl.c_str();
		} else if(em_pm_playmode_all_cycle == g_music_playmode) {
			if(event_callback != NULL) {
				event_callback(em_pm_playertype_music, em_pm_playerevent_onefinish);
			}
			left = g_musiclist.size();
			g_musiclist_playindex++;
			string playurl = g_musiclist[g_musiclist_playindex];
			playurl_cstr = playurl.c_str();
		} else if(em_pm_playmode_random == g_music_playmode) {
			if(event_callback != NULL) {
				event_callback(em_pm_playertype_music, em_pm_playerevent_onefinish);
			}
			g_musiclist_playindex = rand() % g_musiclist.size() ;
			string playurl = g_musiclist[g_musiclist_playindex];
			playurl_cstr = playurl.c_str();
		}

		if(playurl_cstr != NULL) {
			aml_gstplayer_play(g_music_pgstplayer, playurl_cstr);
		}

	}
}


static void notice_gstplayer_callback(st_gstplayer *pgstplayer, aml_gstplayer_event event, void *param)
{
	LOG(LEVEL_INFO, "event:%d param:%p", event, param);

	if(event_callback == NULL) {
		return;
	}

	if(EM_GSTPLAYER_EVENT_PlayFinished == event) {
		event_callback(em_pm_playertype_notice, em_pm_playerevent_allfinish);
	} else if(EM_GSTPLAYER_EVENT_PlayError == event) {
		event_callback(em_pm_playertype_notice, em_pm_playerevent_playfailed);
	}
}

static void tts_player_callback(ttsplay_event event, void *eventparam, void *userparam)
{
	LOG(LEVEL_INFO, "event:%d eventparam:%p userparam:%p", event, eventparam, userparam);

	em_pm_playertype type = g_cur_player_type;

	if(em_pm_playertype_tts == type) {
		if(event_callback == NULL) {
			return;
		}

		if(em_ttsplay_event_finish == event) {
			event_callback(em_pm_playertype_tts, em_pm_playerevent_allfinish);
		} else if(em_ttsplay_event_error) {
			event_callback(em_pm_playertype_tts, em_pm_playerevent_playfailed);
		}
		return;
	}

	if(em_pm_playertype_txtbook == type) {
		if(g_textbooklist.size() > 0) {
			vector<string>::iterator k = g_textbooklist.begin();
			g_textbooklist.erase(k);
		}

		int left = g_textbooklist.size();

		if(em_ttsplay_event_finish == event) {
			if(left == 0) {
				if(event_callback != NULL) {
					event_callback(em_pm_playertype_txtbook, em_pm_playerevent_allfinish);
				}
			} else {
				string playurl = g_textbooklist[0];
				g_pttsplayer->play(playurl.c_str());
			}
		} else if(em_ttsplay_event_error == event) {
			g_textbooklist.clear();
			if(event_callback != NULL) {
				event_callback(em_pm_playertype_txtbook, em_pm_playerevent_playfailed);
			}
		}
		return;
	}
}


void playermanager_play(em_pm_playertype type)
{
	//stop or pause current xxx_player
	playermanager_pause();
	LOG(LEVEL_INFO, "type %d!", type);

	if(em_pm_playertype_music == type) {

		LOG(LEVEL_ERROR, "play list size:%d", g_musiclist.size());
		if(g_musiclist.size() < 0) {
			return;
		}

		string playurl = g_musiclist[g_musiclist_playindex];
		if(g_music_pgstplayer == NULL) {
			g_music_pgstplayer = aml_gstplayer_init(music_gstplayer_callback, NULL);
		}

		if(g_music_pgstplayer != NULL) {
			LOG(LEVEL_INFO, "start to play:%s", playurl.c_str());
			aml_gstplayer_play(g_music_pgstplayer, playurl.c_str());
		} else {
			LOG(LEVEL_FATAL, "g_music_pgstplayer aml_gstplayer_init failed!");
		}

		g_cur_player_type = em_pm_playertype_music;
		return;
	}


	if(em_pm_playertype_notice == type) {
		if( g_notice_content.length() > 0) {
			if(g_notice_pgstplayer == NULL) {
				g_notice_pgstplayer = aml_gstplayer_init(notice_gstplayer_callback, NULL);
			}

			if(NULL == g_notice_pgstplayer){
				LOG(LEVEL_FATAL, "g_music_pgstplayer is NULL!");
				return;
			}
			
			if(!path_is_pcm(g_notice_content)) {
				aml_gstplayer_play(g_notice_pgstplayer, g_notice_content.c_str());
				g_notice_content.clear();

			} else {
				const char *path = g_notice_content.c_str();
				const char *abs_path = path;

				if(path == strstr(path, FILE_PATH_HEAD)) {
					abs_path = path + strlen(FILE_PATH_HEAD);
				}

				const char *wav_path = create_wav_from_pcm(abs_path,
				                       notice_pcm_channels, notice_pcm_samplerate, notice_pcm_format);

				if(NULL != wav_path) {
					char fullpath[256];
					snprintf(fullpath, sizeof(fullpath) - 1, "%s%s", FILE_PATH_HEAD, wav_path);
					aml_gstplayer_play(g_notice_pgstplayer, fullpath);
					g_notice_content.clear();
				} else {
					LOG(LEVEL_FATAL, "create_wav_from_pcm failed!");
				}
			}
			g_cur_player_type = em_pm_playertype_notice;
		}
		return;
	}

	if(em_pm_playertype_tts == type) {
		LOG(LEVEL_INFO, "len %d!", g_tts_content.length());
		if(g_pttsplayer != NULL && g_tts_content.length() > 0) {
			g_pttsplayer->setcallback(tts_player_callback, (void*)NULL);
			g_pttsplayer->init(NULL);
			g_pttsplayer->play(g_tts_content.c_str());
			g_tts_content.clear();
			g_cur_player_type = em_pm_playertype_tts;
		}
		return;
	}

	if(em_pm_playertype_txtbook == type) {
		if(g_pttsplayer != NULL && g_textbooklist.size() > 0) {
			g_pttsplayer->setcallback(tts_player_callback, (void*)NULL);
			g_pttsplayer->init(NULL);

			string playurl = g_textbooklist[0];
			g_pttsplayer->play(playurl.c_str());
			g_cur_player_type = em_pm_playertype_txtbook;
		}
		return;
	}
}


void playermanager_pause()
{
	int type = g_cur_player_type;

	if(em_pm_playertype_music == type) {
		if(g_music_pgstplayer != NULL) {
			aml_gstplayer_status status = aml_gstplayer_get_status(g_music_pgstplayer);
			if(EM_GSTPLAYER_STATUS_PLAYING == status) {
				aml_gstplayer_pause(g_music_pgstplayer);
				LOG(LEVEL_INFO, "music player paused!");
			}
		}
	}

	if(em_pm_playertype_notice == type ) {
		if(g_notice_pgstplayer != NULL) {
			aml_gstplayer_stop(g_notice_pgstplayer);
			LOG(LEVEL_INFO, "notice player stopped!");
		}
	}

	if(em_pm_playertype_tts == type ) {
		if(g_pttsplayer != NULL) {
			g_pttsplayer->stop();
			LOG(LEVEL_INFO, "tts player stopped!");
		}
	}

	if(em_pm_playertype_txtbook == type) {
		if(g_pttsplayer != NULL) {
			g_pttsplayer->stop();
			LOG(LEVEL_INFO, "txtbook player stopped!");
		}
	}

	g_cur_player_type = em_pm_playertype_reserver;
}


void playermanager_resume(em_pm_playertype type)
{
	playermanager_pause();

	if(em_pm_playertype_notice == type || em_pm_playertype_tts == type) {
		LOG(LEVEL_WARN, "notice and tts not supports operation resume!");
		return;
	}

	if(em_pm_playertype_music == type) {
		if(g_music_pgstplayer != NULL) {
			aml_gstplayer_status status = aml_gstplayer_get_status(g_music_pgstplayer);
			if(EM_GSTPLAYER_STATUS_PAUSE == status) {
				aml_gstplayer_resume(g_music_pgstplayer);
				g_cur_player_type = em_pm_playertype_music;
			} else {
				if(g_musiclist.size() > 0) {
					string playurl = g_musiclist[g_musiclist_playindex];
					aml_gstplayer_play(g_music_pgstplayer, playurl.c_str());
					g_cur_player_type = em_pm_playertype_music;
				}
			}
		}
		return;
	}

	if(em_pm_playertype_txtbook == type && g_pttsplayer != NULL) {
		if(g_textbooklist.size() > 0) {
			string playurl = g_textbooklist[0];
			g_pttsplayer->init(NULL);
			g_pttsplayer->play(playurl.c_str());
			g_cur_player_type = em_pm_playertype_txtbook;
		}
		return;
	}
}


void playermanager_stop(em_pm_playertype type)
{
	if(em_pm_playertype_music == (type & em_pm_playertype_music)) {
		if(g_music_pgstplayer != NULL) {
			aml_gstplayer_status status = aml_gstplayer_get_status(g_music_pgstplayer);
			if(EM_GSTPLAYER_STATUS_PAUSE == status || EM_GSTPLAYER_STATUS_PLAYING == status) {
				aml_gstplayer_stop(g_music_pgstplayer);
				g_musiclist.clear();
				g_musiclist_playindex = 0;
			}
			aml_gstplayer_uninit(g_music_pgstplayer);
			g_music_pgstplayer = NULL;
		}
	}

	if(em_pm_playertype_notice == (type & em_pm_playertype_notice)) {
		if(g_notice_pgstplayer != NULL) {
			aml_gstplayer_stop(g_notice_pgstplayer);
			aml_gstplayer_uninit(g_notice_pgstplayer);
			g_notice_pgstplayer = NULL;
		}
	}

	if(em_pm_playertype_tts == (type & em_pm_playertype_tts)) {
		if(g_pttsplayer != NULL) {
			g_pttsplayer->stop();
		}
	}

	if(em_pm_playertype_txtbook == (type & em_pm_playertype_txtbook)) {
		if(g_pttsplayer != NULL) {
			g_pttsplayer->stop();
		}
		g_textbooklist.clear();
	}
}
