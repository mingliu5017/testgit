/*
* This file is part of avs.
* Copyright (c) amlogic 2017
* All rights reserved.
* author:renjun.xu@amlogic.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software.
*
*/
#ifndef __EVENTS_H__
#define __EVENTS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h> 

#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })

struct inputevent{
   	struct timeval time;
	__u16 type;
	__u16 code;
	__u32 value;
};

// TODO: move these over to std::function.
typedef int (*ev_callback)(int fd, uint32_t epevents);
typedef int (*ev_set_key_callback)(int code, int value);

int ev_init(ev_callback input_cb);
void ev_exit();

// 'timeout' has the same semantics as poll(2).
//    0 : don't block
//  < 0 : block forever
//  > 0 : block for 'timeout' milliseconds
int ev_wait(int timeout);

int ev_get_input(int fd, uint32_t epevents, struct inputevent* ev);
void ev_dispatch();
int ev_get_epollfd();

#ifdef __cplusplus
}
#endif

#endif
