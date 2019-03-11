/*
 * =====================================================================================
 *
 *       Filename:  http.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2017年08月29日 11时34分40秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  huangzhenjiang , huangzhenjiang@cmhi.chinamobile.com
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

char * Mygettime()
{
    time_t timep;
    time(&timep);
    return asctime(gmtime(&timep));
}

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  //struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  printf("WriteCallback:\n%s\n",(char *)contents);
  return realsize;
}


int httppostcallback(char * url, char * user_key, char * PostData)
{
  CURL *curl;
  CURLcode res;
  char tmp_buf[100]={0};

  struct curl_slist *list = NULL;

  struct MemoryStruct chunk;

  if (url == NULL || PostData == NULL)
  {
	printf("%s: no data or url is null.\n",__func__);
	return -1; 
  }
	printf("%s: in data:\n%s\n",__func__,PostData);
  printf("curl_version:%s\n",curl_version());
  /* In windows, this will init the winsock stuff */
  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  /* Check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n",
            curl_easy_strerror(res));
    return 1;
  }

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
	/* get verbose debug output please */
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

	/* First set the URL that is about to receive our POST. */
    curl_easy_setopt(curl, CURLOPT_URL, url);
	/*SKIP_PEER_VERIFICATION*/
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	/*SKIP_HOSTNAME_VERIFICATION*/
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    /* Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostData);
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
     /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
        itself */
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(data));

    list = curl_slist_append(list, "Content-Type: application/json");
    list = curl_slist_append(list, "Accept-Charset: utf-8");
	if (user_key != NULL)
	{
		sprintf(tmp_buf,"User-Key: %s",user_key);
		list = curl_slist_append(list, tmp_buf);
	}

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    //curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    curl_slist_free_all(list); /* free the list again */
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}

