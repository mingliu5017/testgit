/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef LOG_H_
#define LOG_H_

#define GAT_CRITICAL    0X00
#define GAT_ERROR       0x01
#define GAT_WARNING     0X02
#define GAT_NOTIC       0X03
#define GAT_DEBUG       0x04
#define GAT_DUMP        0x05

#define GAT_LOGLEVEL GAT_DEBUG

#define LOGSYSTEM 0

#if LOGSYSTEM
#define Log(format, args...)  log_debug( format, ##args )
#else
#define Log(format, args...)  printf( format, ##args ) 
#endif

#define dumpPrintf(format, args...)  \
{\
    if(GAT_LOGLEVEL==GAT_DUMP ) \
        Log( format, ##args ); \
}

#define dumpBuf( level,buf,len )\
{\
    if(GAT_LOGLEVEL>=level)\
    {   int32 i;\
        Log("***********************************************************\n");\
        for( i=0;i<len;i++ )\
        {\
         if( i!=0&&0==i%20 ) \
            Log("\r\n");\
         Log("%02x ",*((uint8*)(buf+i)) );\
        }\
        Log("\r\n\r\n");\
    }\
}

#define gatPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[gat] "format, ##args);\
}
#define evtPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[evt] "format, ##args);\
}
#define amlserPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[aml server] "format, ##args);\
}

#define tcpPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[tcp] "format, ##args);\
}

#define acePrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[ace] "format, ##args);\
}
#define jsonPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[json] "format, ##args);\
}

#define timerPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[timer] "format, ##args);\
}

#define netPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[netCheck] "format, ##args);\
}
#define splayprintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[splay]"format, ##args);\
}
#define miguPrintf(level,format, args...)\
{\
    if(GAT_LOGLEVEL>=level) \
        Log( "[migu] "format, ##args);\
}

#endif
