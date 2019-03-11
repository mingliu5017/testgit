/***************************************************************************
** CopyRight: Amlogic
** Author   : ming.liu@amlogic.com
** Date     : 2018-09-28
** Description
**
***************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include "key_event.h"
#include "key_event_process.h"

#if 0 //for migu pad
#define WAIT_KEY_TIMEOUT_SEC    120
#define KEY_EVENT_TIME_INTERVAL 20
#else
#define WAIT_KEY_TIMEOUT_SEC    10//10 //120
#define KEY_EVENT_TIME_INTERVAL 1 //1 //20
#endif

#define DEFAULT_KEY_NUM 1

struct key_timer_t {
    int key_code;
    int count;
};

struct KeyMapItem_t {
    const char* type;
    int value;
};

pthread_mutex_t key_queue_mutex;
pthread_cond_t key_queue_cond;
int key_queue[256], key_queue_len;
struct timeval last_queue_time;
char key_pressed[KEY_MAX + 1];
int key_last_down;
int key_down_count;
int enable_reboot;
int rel_sum;

int last_key;
int report_longpress_flag;

int num_keys;
struct KeyMapItem_t* keys_map;
pthread_t input_thread_;

static void EnqueueKey(int key_code);

static struct KeyMapItem_t g_default_keymap[] = {
    { "mute", KEY_RIGHT },
};

static void load_key_map() {
    FILE* fstab = fopen("/etc/gpio_key.kl", "r");
    if (fstab != NULL) {
        printf("loaded /etc/gpio_key.kl\n");
        int alloc = 2;
        keys_map = (struct KeyMapItem_t*)malloc(alloc * sizeof(struct KeyMapItem_t));
        num_keys = 0;

        char buffer[1024];
        int i;
        int value = -1;
        while (fgets(buffer, sizeof(buffer)-1, fstab)) {
            for (i = 0; buffer[i] && isspace(buffer[i]); ++i);

            if (buffer[i] == '\0' || buffer[i] == '#') continue;

            char* original = strdup(buffer);
            char* type = strtok(original+i, " \t\n");
            char* key = strtok(NULL, " \t\n");
            value = atoi (key);
            if (type && key && (value > 0)) {
                while (num_keys >= alloc) {
                    alloc *= 2;
                    keys_map = (struct KeyMapItem_t*)realloc(keys_map, alloc*sizeof(struct KeyMapItem_t));
                }
                keys_map[num_keys].type = strdup(type);
                keys_map[num_keys].value = value;

                ++num_keys;
            } else {
                printf("error: skipping malformed keyboard.lk line: %s\n", original);
            }
            free(original);
        }

        fclose(fstab);
    } else {
        printf("failed to open /etc/gpio_key.kl, use default map\n");
        num_keys = DEFAULT_KEY_NUM;
        keys_map = g_default_keymap;
    }

    printf("keyboard key map table:\n");
    int i;
    for (i = 0; i < num_keys; ++i) {
        struct KeyMapItem_t* v = &keys_map[i];
        printf("  %d type:%s value:%d\n", i, v->type, v->value);
    }
}

static int getMapKeyVal(int key) {
    int i;
    for (i = 0; i < num_keys; i++) {
        struct KeyMapItem_t* v = &keys_map[i];
        if (v->value == key)
            return v->value;
    }

    return -1;
}

static const char* getKeyType(int key) {
    int i;
    for (i = 0; i < num_keys; i++) {
        struct KeyMapItem_t* v = &keys_map[i];
        if (v->value == key)
            return v->type;
    }

    return NULL;
}

static void KeyLongPress(int key) {
    const char* keyType=getKeyType(key);
    if (keyType != NULL) {
        report_longpress_flag=1;
		EnqueueKey(key);
	}
}

static void time_key(int key_code, int count) {
    int long_press = 0;

	#if 0//for migu pad
	if (key_code == 200) //mute
		sleep(10); //10
	else if (key_code == 201) //pause
		sleep(5); //5
	else sleep(3); //3
    #endif
	pthread_mutex_lock(&key_queue_mutex);
    if (key_last_down == key_code && key_down_count == count) {
        long_press = 1;
    }
    pthread_mutex_unlock(&key_queue_mutex);
    if (long_press)
    KeyLongPress(key_code);
}

void* time_key_helper(void* cookie) {
    struct key_timer_t* info = (struct key_timer_t*) cookie;
    time_key(info->key_code, info->count);
    free(info);
    return NULL;
}

static long checkEventTime(struct timeval *before, struct timeval *later) {
    time_t before_sec = before->tv_sec;
    suseconds_t before_usec = before->tv_usec;
    time_t later_sec = later->tv_sec;
    suseconds_t later_usec = later->tv_usec;

    long sec_diff = (later_sec - before_sec) * 1000;
    if (sec_diff < 0)
        return 1;

    long ret = sec_diff + (later_usec - before_usec) / 1000;
    if (ret >= KEY_EVENT_TIME_INTERVAL)
        return 1;

    return 0;
}

static void EnqueueKey(int key_code) {
    struct timeval now;
    gettimeofday(&now, NULL);

    pthread_mutex_lock(&key_queue_mutex);
    const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
    if (key_queue_len < queue_max) {
        if (last_key != key_code || checkEventTime(&last_queue_time, &now)) {
            key_queue[key_queue_len++] = key_code;
            last_queue_time = now;
        }
        pthread_cond_signal(&key_queue_cond);
    }
    pthread_mutex_unlock(&key_queue_mutex);
}

static void ProcessKey(int key_code, int value) {
    int register_key = 0;

    pthread_mutex_lock(&key_queue_mutex);
    key_pressed[key_code] = value;
    if (value == 1) {/*1:key down*/
        ++key_down_count;
        key_last_down = key_code;
        struct key_timer_t* info = (struct key_timer_t*)malloc(sizeof(struct key_timer_t));
        //info->ep = this;
        info->key_code = key_code;
        info->count = key_down_count;
        pthread_t thread;
        pthread_create(&thread, NULL, &time_key_helper, info);
        pthread_detach(thread);
    } else if(value == 2){/*2:key repeat*/
    } else {/*0:key down*/
        if (key_last_down == key_code) {
            register_key = 1;
        }
        key_last_down = -1;
    }
    pthread_mutex_unlock(&key_queue_mutex);
    last_key = key_code;
    if (register_key)
	{
		if (report_longpress_flag == 1)
			report_longpress_flag=0;
		else
			EnqueueKey(key_code);
	}
}


static int OnInputEvent(int fd, uint32_t epevents) {
    struct inputevent ev;

    if (ev_get_input(fd, epevents, &ev) == -1) {
        return -1;
    }
    if (ev.type == EV_SYN) {
        return 0;
    }
    if (ev.type == EV_KEY && ev.code <= KEY_MAX) {
        int code = getMapKeyVal(ev.code);
        if (code > 0)
            ProcessKey(code, ev.value);
        else
            ProcessKey(ev.code, ev.value);
    }
    return 0;
}

static int InputCallback(int fd, uint32_t epevents) {
    return OnInputEvent(fd, epevents);
}

static void* InputThreadLoop(void* p) {
    while (1) {
        if (!ev_wait(-1))
            ev_dispatch();
    }
    return NULL;
}

char* WaitKey(int* flag) {
    char* keyValue = NULL;
    int rc;
    int key = -1;
    char* keyType = NULL;

    pthread_mutex_lock(&key_queue_mutex);
    do {
        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = now.tv_usec * 1000;
        timeout.tv_sec += WAIT_KEY_TIMEOUT_SEC;
		rc = 0;
        while (key_queue_len == 0 && rc != ETIMEDOUT)
            rc = pthread_cond_timedwait(&key_queue_cond, 
			&key_queue_mutex, &timeout);
    } while (key_queue_len == 0);

    if (key_queue_len > 0) {
        key = key_queue[0];
        memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
    }
    pthread_mutex_unlock(&key_queue_mutex);
    keyType=(char*)getKeyType(key);
    if (keyType != NULL) {
        if (report_longpress_flag == 1) {
	    keyValue = keyType;
            *flag = 1; 
	} else {
	    keyValue = keyType;
            *flag = 0; 
	}
        //report_longpress_flag=0;
    }
    return keyValue;
}

int key_eventprocess_init(void)
{
    key_queue_len = 0;
    key_last_down = -1;
    key_down_count = 0;
    last_key = -1;
    report_longpress_flag = 0;
    num_keys = 0;
    keys_map = NULL;
    pthread_mutex_init(&key_queue_mutex, NULL);
    pthread_cond_init(&key_queue_cond, NULL);
    memset(key_pressed, 0, sizeof(key_pressed));
    memset(&last_queue_time, 0, sizeof(last_queue_time));
    load_key_map();
    ev_init(InputCallback);
    pthread_create(&input_thread_, NULL, InputThreadLoop, NULL);
    return 0;
}

