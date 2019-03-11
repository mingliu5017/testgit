#ifndef __MIGU_H
#define __MIGU_H

#ifdef __cplusplus
extern "C"
{
#endif

char * getSdkVersion(char * version);
int migusdk_init( char * device_info);

/*歌曲id查询歌曲信息接口*/
char * get_musicInfo(char * musicId, char *result);
/*关键字搜索歌曲*/
char * get_musicInfo_key(char * keybuf, char *result);
/*查询音乐歌单下歌曲*/
char * get_musicInfo_musicSheetId(char * musicSheetId, char *result);
/*关键字搜索音乐信息*/
char * get_musicInfo_newKey(char * text, char * type, int  pageIndex, int  pageSize, int searchType, int issemantic, int isCorrect, char *result);
/*查询所有标签接口*/
char * get_musicInfo_tag( char *result);
/*标签id查询歌曲接口（分页查询)*/
char * get_musicInfo_musicTag(char * tagId, int startNum , int endNum, char *result);

#ifdef __cplusplus
}
#endif

#endif
