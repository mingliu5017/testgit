/*
 *  * sai_sdk.h
 * */
#ifndef SAI_SDK_H_
#define SAI_SDK_H_

#define SAI_SDK_API __attribute__((visibility("default")))

#if defined(__cplusplus)
extern "C"
{
#endif

////
////Log level for release edition
////
typedef enum releaseLogLevel {
    SAI_LOGGER_CRITICAL,
    SAI_LOGGER_WARN,
    SAI_LOGGER_INFO,
    SAI_LOGGER_DEBUG,
    SAI_LOGGER_OFF
} releaseLogLevel;

////
////Set logger level
////@param l: logger level
////
SAI_SDK_API void set_logger_level(releaseLogLevel l);

////
////Set size of logger file
////@param num_file: number of splited files
////@param per_flen_bytes: limit(bytes) of each file length
////
SAI_SDK_API void set_logger_size(size_t num_file, size_t per_flen_bytes);

#if defined(ENABLE_HTTP2)
////
////Set time out threshold for each data transfer
////@param ms: time(milliseconds)
////@return true means success, false otherwise
////
SAI_SDK_API int set_fragment_data_transfer_timeout(int ms);
#endif

#if defined(ENABLE_HTTP2) || defined(ENABLE_SAINET)
////
////Status of VAD
////
typedef enum {
    EVENT_VAD_NORMAL,//vad middle status
    EVENT_VAD_BEGIN,//vad begin flag
    EVENT_VAD_END,//vad end edge flag
    EVENT_VAD_BEGIN_TIMEOUT,//vad timeout due to no begin
    EVENT_VAD_END_TIMEOUT//vad timeout due to too long voice
} VADEventStatus;

////
////Set timeout value of vad begin
////@param b: timeout value (ms)
////
SAI_SDK_API void set_vad_begin_timeout(int b);

////
////Set timeout value of tolerable vad voice length
////@param e: timeout value (ms)
////
SAI_SDK_API void set_vad_end_timeout(int e);

////
////Network status code
////
typedef enum networkStatusCode {
  SLOW_SPEED,
  VERY_SLOW_SPEED,
  CONNECTED,
  RECONNECTING,
  CLOSED,
  SHUTDOWN,
  IDLE,
  RECOVERED
} networkStatusCode;

typedef enum netEventType {
  SOUNDAI_FINAL_RESULT, //ASR final result
  SOUNDAI_TEMP_RESULT, //ASR temp result
  SOUNDAI_ERROR, //Error event
  SOUNDAI_VAD, //VAD event
  SOUNDAI_SEMANTICS
} netEventType;

typedef enum codeInfo {
  SOUNDAI_ASR_ENGINE_ERROR, //ASR engine error
  SOUNDAI_ALL, //for reservation
  SOUNDAI_OK, //normal status
  SOUNDAI_TIMEOUT, //timeout
  SOUNDAI_BAD_REQUEST, //bad request
  SOUNDAI_SERVER_ERROR, //server error, see msg for details
  SOUNDAI_ASR_FINAL_RESULT,
  SOUNDAI_ASR_TEMP_RESULT
} codeInfo;

////
////Call-back functional handler
////@param id: dialog id, type: event type, error_code: network error code, msg: information message
////
typedef void (*net_event_callback)(const char * id, int type, int error_code, const char* msg);

////
////Call-back functional handler
////@param code: network status, msg: information message
////
typedef void (*net_status_callback)(networkStatusCode code, const char* msg);

////
////Call-back functional handler
////@param id: dialog id
////@param len: sent data length (ms)
////
typedef void (*beat_data_callback)(const char * id, size_t len);

#endif

////
////Activity indicator of voice-print verification
////
typedef enum VoicePrintActivity {
    SAI_VP_REG_SUCCESS = 5,
    SAI_VP_REG_MORE,
    SAI_VP_VERIFY_FAILED
} VoicePrintActivity;

////
////External error code
////
typedef enum ExternalError {
    SAI_NO_RSLT_RECEIVED = 1102,
    SAI_BAD_DRIVER_DATA,
    SAI_BAD_RES_ALLOC,
    SAI_USER_FAIL
} ExternalError;

////
////Satrt voice recording and processing for voice-print verification
////
SAI_SDK_API void start_voice_print_verification();

////
////Call-back functional handler
////@param val: voice energy value
////
typedef void (*wav_energy_callback)(float val);

////
////Call-back functional handler
////@param is_one_shot: normally, it denotes whether the wakeup is in one-shot-mode(1:yes, 0:no);
////                    when voice-print verification is on, its value tells:
////   SAI_VP_REG_SUCCESS: success, verification runs smoothly
////   SAI_VP_REG_MORE: need more time for voice recording
////   SAI_VP_VERIFY_FAILED: failure
////also, when some error took place, this handler provides error code like ExternalError defined above
////
typedef void (*oneshot_callback)(int event_type);

////
////Call-back functional handler
////@param buffer: voip data (currently mono-channel, 16KHz)
////@param size: buffer length
////
////Note: When set up voip-mode, you would be feed with this voip data, copy it for further usage
typedef void (*voip_data_callback)(void * usr_data_voip, const char* id, const char *buffer, size_t size);

////
////Call-back functional handler
////@param id: dialog id
////@param buffer: wakeup data (currently mono-channel, 16KHz) for transfering
////@param size: buffer length
////
////Note: When you got this wakeup data, copy it for transfering to sai wake-learning online server
typedef void (*ivw_data_callback)(const char * id, const char *buffer, size_t size);

////
////Call-back functional handler
////@param id: dialog id
////@param buffer: preprocessed asr data for transfering
////@param size: buffer length
////
////Note: When you got data, copy it for transfering to sai online server for postprocessing
typedef void (*asr_data_callback)(void * usr_data_asr, const char * id, const char *buffer, size_t size);


////
////Call-back functional handler
////@param id: dialog id
////@param key_word: key word
////@param score: wake belief level
////@param wakeup_result: wake keyword index
////@param angle: angle value
////@param usr_set: denoting if wakeup is by intentional user setting
////@return 0 denotes user allows sdk to be waken up
////Note: When sdk is waken up, this functional handler is called back.
////When `id` and `key_word` are empty, user should set the return value to let sdk truly wake up (generate new dialog id) or not
typedef int (*wakeup_callback)(void * usr_data_wk, const char * id, const char * key_word,
                                float score, int wakeup_result, float angle, int usr_set);

#ifdef ENABLE_DRIVER_DATA
////
////Feed data to sdk instead of using the correspoding call back interface
////@note When this api is utilized, set driver_data_callback as nullptr to shut down the call back
////@param buf: data buffer
////@param size: buffer size
////
SAI_SDK_API void feed_driver_data(const char *buf, size_t size);

////
////Call-back functional handler
////@brief got data from driver and provided for upper usage
////@param ctx: user defined hook-data
////@param buf: multi-channel audio data buffer, data format is like ch0|ch1|...|chn|ch0|ch1|...|chn...
////@param size: length of above data buffer
typedef size_t (*driver_data_callback)(void *ctx, int16_t *buf, size_t size);

////
////Initialize data system
////@param info: client information text-string, got from sai provisioner
////@param threshold: wakeup threshold (0~1)
////@param config_file_path: resource file path, e.g., "/etc/sai_config"
////@param asr_cbf, w_cbf, voip_cbf: call-back handlers for asr data, wakeup, voip data, others likewise
////@return 0 successful, error code otherwise
SAI_SDK_API int init_system(const char * info, double threshold, const char * config_file_path,
                asr_data_callback asr_cbf, wakeup_callback w_cbf, ivw_data_callback ivw_cbf,
                voip_data_callback voip_cbf, oneshot_callback one_shot_cbf, wav_energy_callback energy_cbf,
#if defined(ENABLE_HTTP2) || defined(ENABLE_SAINET)
                net_event_callback event_cbf,
                net_status_callback net_cb,
                beat_data_callback beat_cb,
#endif
                driver_data_callback driver_cb,
                void * usr_data_asr, void * usr_data_wk, void * usr_data_voip, void * d_usr_data);
#else
////
////Initialize data system
////@param info: client information text-string, got from sai provisioner
////@param threshold: wakeup threshold (0~1)
////@param config_file_path: resource file path, e.g., "/etc/sai_config"
////@param asr_cbf, w_cbf, voip_cbf: call-back handlers for asr data, wakeup, voip data, others likewise
////@return 0 successful, error code otherwise
SAI_SDK_API int init_system(const char * info, double threshold, const char * config_file_path,
                asr_data_callback asr_cbf, wakeup_callback w_cbf, ivw_data_callback ivw_cbf,
                voip_data_callback voip_cbf, oneshot_callback one_shot_cbf, wav_energy_callback energy_cbf,
#if defined(ENABLE_HTTP2) || defined(ENABLE_SAINET)
                net_event_callback event_cbf,
                net_status_callback net_cb,
                beat_data_callback beat_cb,
#endif
                void * usr_data_asr, void * usr_data_wk, void * usr_data_voip);
#endif

////
////Start service
////@return 0 successful, error code otherwise
////
////
SAI_SDK_API int start_service();

////
////Stop service
////@return 0 successful, error code otherwise
////
SAI_SDK_API int stop_service();

////
////Terminate & release system resources
////
SAI_SDK_API void terminate_system();

////
////Tell sdk that its waken-status must stop right now
////
SAI_SDK_API void set_unwakeup_status();

////
////Tell sdk that it has to be waken right now
////
SAI_SDK_API void set_wakeup_status();

////
////Set prefix & suffix (ms) for wakeup pcm audio recording
////
SAI_SDK_API void set_wakeup_prefix_suffix(size_t prefix, size_t suffix);

////
////Set voip mode
////@param flag: 0: normal mode, 1: voip mode
////
SAI_SDK_API void set_voip_flag(int flag);

////
////Set voip beam pattern
////@param flag: 1: fixed direction; 0: non-fixed direction
////
SAI_SDK_API void set_voip_beam(int flag);

////
////Set vivid thresholds for each wake word
////@param th: threshold value
////@param index: key word index
////
SAI_SDK_API void set_ivw_threshold(float th, int index);

////
////Wake mode by day/night
////
typedef enum WakeMode {
  SOUNDAI_DAY,
  SOUNDAI_NIGHT
} WakeMode;

////
////Set wake mode
////@param mode: wake mode
////
SAI_SDK_API void set_wake_mode(WakeMode mode);

////
////Enable low-power mode
////@param val: 1 denotes setting sdk into low-power-consumption mode, 0 otherwise
////
SAI_SDK_API void enable_low_power_mode(int val);

////
////Shut one key word
////@param w: key word index
////
SAI_SDK_API void shut_wake_word(int w);

////
////Get key word list size
////@return size of the key word list
////
SAI_SDK_API int get_key_word_list_size();

////
////Get key word of given index
////@param index: index value
////@return corresponding key word
////
SAI_SDK_API const char * get_key_word_at(int index);

////
////Set beamforming angle
////@param value: angle value
////
SAI_SDK_API void set_talking_angle(float value);

////
////Try to set led with element-wise led-light-control
////@return 0 successful
////
SAI_SDK_API int set_led_lights(unsigned int leds);

////
////Try to set led with angle
////@param angle: angle value
////@return 0 successful
////
SAI_SDK_API int set_led_angle(float angle);

////
////Try to set led with pre-defined led-light-mode
////@param mode:
////     SAILED platform
/*         - 0 all blue(default)
 *         - 1 all dimmed
 *         - 2 start working
 *         - 3 ActiveListening
 *         - 4 Thinking
 *         - 5 Speaking
 *         - 6 Microphones Off
 *         - 7 ALL ORIGIN
 *         - 8 ALL CYAN
 *     3229 platform
 *         - 0 all blue (default)
 *         - 1 all dimmed
 *         - 2 all green
 *         - 3 all red
 *     other platform
 *         - 0 all lights (default)
 *         - 1 all dimmed
 *         - 2 even lights
 *         - 3 odd lights
*////
////@return 0 successful
////
SAI_SDK_API int set_led_mode(int mode);

SAI_SDK_API int set_led_listening(float angle);

SAI_SDK_API int set_led_color(unsigned int led, unsigned int color);

SAI_SDK_API int set_led_volume(float percent);

////
////Set user's external data for transfering to online server
////@param data: buffer
////@param len: size
////
SAI_SDK_API void set_opaque_data(const char * data, size_t len);

////
////Get client id
////@return current client id
////
SAI_SDK_API const char *get_client_id();

////
////on response body received
////@param id: dialog id
////@param content_type: data type
////@param buffer: response body data
////@param size: buffer length
////
SAI_SDK_API void on_result_received(const char *id,
                        const char *content_type,
                        const char* buffer,
                        size_t size);

////
////Set sdk into (un-)mute status
////@param val: 1(mute), 0(un-mute)
////
SAI_SDK_API void set_mute_mode(int val);

#if defined(__cplusplus)
}
#endif

#endif //SAI_SDK_H_
