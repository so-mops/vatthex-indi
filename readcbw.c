/*############################################################################
#  Title: readcbw.c
#  Author: Chris Johnson
#  Date: 4/20/2023
#  Description: communications routine for getting temps from Control By Web.
#
#
#############################################################################*/


#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "mjson.h"
#include <string.h>
#include "readcbw.h"

#define NOTEMP  999.9
#define CBWADDRESS 10.0.3.23

static char oneWireSensor1[10], oneWireSensor2[10];
int status;

struct MemoryStruct {
  char *memory;
  size_t size;
};

static const struct json_attr_t json_attrs[] = {
    {"oneWireSensor1",   t_string, .addr.string = oneWireSensor1, .len = sizeof(oneWireSensor1)},
    {"oneWireSensor2",   t_string, .addr.string = oneWireSensor2, .len = sizeof(oneWireSensor2)},
    {"", t_ignore, NULL},
    {NULL},
};


/*############################################################################
#  Title: WriteMemoryCallback
#  Author: C.Johnson
#  Date: 4/20/2023
#  Args: 
#	 
#  Returns: ???
#  Description: shamelessly stolen from libcurl example on internet
#
#############################################################################*/
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    printf("error: not enough memory\n");
    return 0;
  }
mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

/*############################################################################
#  Title: GetStrutTempCBW
#  Author: C.Johnson
#  Date: 4/20/2023
#  Args:  pointer to char array containing errors
#	 
#  Returns: ???
#  Description: uses libcurl and microJSON to quickly get temps from
#      Control By Web.  This gets 2 temps from the cbw, averages, and returns
#	one double.
#
#	CSJ-5/25/2023  -  changed to average 2 temps and return single temp.
#
#############################################################################*/
//int GetStrutTempCBW(tempstruct *mytemps)
double GetStrutTempCBW(char *myerr)
{
double returnTemp;
CURL *curl;
  CURLcode res;
struct MemoryStruct chunk;
  chunk.memory = malloc(450);  
  chunk.size = 0; 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "10.0.3.23/state.json");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
     
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
              
  }



status = json_read_object(chunk.memory, json_attrs, NULL);


if (status != 0)
	{
        sprintf(myerr, "%s",json_error_string(status));
	returnTemp = NOTEMP;
	}
else
	{
        sprintf(myerr, " ");
	returnTemp = (atof(oneWireSensor1)+atof(oneWireSensor2))/2;
	}


    /* always cleanup */
curl_easy_cleanup(curl);
free(chunk.memory);
return(returnTemp);
}
