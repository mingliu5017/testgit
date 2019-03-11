/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _IOFSTRING_H_
#define _IOFSTRING_H_
#include "gatplatform.h"

#define TESTING_MEMORY 0

#if TESTING_MEMORY
char *mallocP;
#define iofFree(addr)                   printf("[..FREE %p] %s[%d]\n",addr,__FUNCTION__,__LINE__);free(addr)
#define iofMalloc(len)                  (mallocP=malloc(len));printf("[..MALLOC %p] %s[%d] len=%d\n",mallocP,__FUNCTION__,__LINE__,len)
#else
#define iofFree(addr)                   free(addr)
#define iofMalloc(len)                  malloc(len)
#endif

#define iofMemset(s,ch,n)               memset(s,ch,n)
#define iofMemcpy(s,d,l)                memcpy(s,d,l)
#define iofMemcmp(s,d,l)                memcmp(s,d,l)

#define iofStrcmp(s1,s2)                strcmp(s1,s2)
#define iofStrncmp(s1,s2,n)             strncmp(s1,s2,n)
#define iofStrstr( haystack,needle)     strstr( haystack,needle)
#define iofStrcpy(s1,s2)                strcpy(s1,s2)
#define iofStrncpy(s1,s2,n)             strncpy(s1,s2,n)
#define iofStrlen( s )                  strlen(s)
#define iofStrtoul(nptr, endptr, base)  strtoul(nptr, endptr, base)
#define iofAtoi(str)                    atoi(str)
#define iofSprintf                      sprintf

#endif
