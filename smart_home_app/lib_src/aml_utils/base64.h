#ifndef _BASE64_H
#define _BASE64_H

#ifdef   __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

char *base64_encode(char *str);

char *base64_decode(char *code);

#ifdef   __cplusplus
}
#endif

#endif

