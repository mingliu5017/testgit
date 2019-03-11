#ifndef _KEY_EVENT_HANDLE_H_
#define _KEY_EVENT_HANDLE_H_

#ifdef __cplusplus
extern "C"{ 
#endif

#include "statusevent_manage.h"

#define LONG_PRESS_FLAG   1
#define SHORT_PRESS_FLAG  0

void keyEventHandle(keyNode_t *pKeyNode);
void key_event_handle_start(void);

#ifdef __cplusplus
}
#endif

#endif 

