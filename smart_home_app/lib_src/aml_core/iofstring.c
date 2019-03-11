/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#include "gatplatformType.h"
#include "gattypes.h"
#include "iofstring.h"

int8 *strnstr(const int8 *p, const int8 *strObj, int32 n)
{
    int32 len = strlen(strObj);
    int32 i = 0;
    int32 j = 0;

    if(len <= 0)
    {
        return (int8 *)p;
    }\
    if(len > n)
    {
        return NULL;
    }

    for(i = 0; i < n; i++)
    {
        if(p[i] == 0)
        {
            return NULL;
        }

        for(j = 0; j < len; j++)
        {
            if(p[i + j] != strObj[j])
                break;
        }
        if(j == len)
        {
            /*  */
            return (int8 *)(p + i);
        }
    }

    return NULL;
}

int32 strToHex(int8 *dest, int8 *src)
{
    int8 h1,h2;
    int8 s1,s2;
    int32 i = 0,j =0;
    int32 len = strlen(src);

    for (i=0; i<len; i++)
    {
        if(0 == (i&0x01))
        {
            h1 = src[i];
        }
        else
        {
            h2 = src[i];

            s1 = toupper(h1) - 0x30;
            if (s1 > 9)
            s1 -= 7;

            s2 = toupper(h2) - 0x30;
            if (s2 > 9)
                s2 -= 7;

            dest[j++] = s1*16 + s2;
        }

    }

    if(j)
        j -= 1;

    return j;
}
void hexToStr( unsigned char *dest,unsigned char *src,int srcLen,char flag )
{
    int32 i=0;
    for( i=0;i<srcLen;i++ )
    {
        if(flag)
        {
            iofSprintf( dest+i*2,"%02X",src[i] );
        }

        else
        {
            iofSprintf( dest+i*2,"%02x",src[i] );
        }
    }
}