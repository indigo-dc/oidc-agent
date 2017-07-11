#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "http.h"
#include "config.h"



void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);

  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);

  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, struct string *pooh)
{

  if(size*nmemb < 1)
    return 0;

  if(pooh->len) {
    *(char *)ptr = pooh->ptr[0]; /* copy one single byte */ 
    pooh->ptr++;                 /* advance pointer */ 
    pooh->len--;                /* less data left */ 
    return 1;                        /* we return 1 byte at a time! */ 
  }

  return 0;                          /* no more data left to deliver */ 
}
const char* httpsGET(const char* url) {
  CURL* curl;
  CURLcode res;

  res = curl_global_init(CURL_GLOBAL_ALL);
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n",
        curl_easy_strerror(res));
    exit(EXIT_FAILURE);
  }

  curl = curl_easy_init();
  struct string s;
  if(curl) {
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_CAPATH, config.cert_path);
 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);  
    printf("%s\n\n",s);
  } 
  else {
    fprintf(stderr, "Couldn't init curl.\n");
    exit(EXIT_FAILURE);
  }
  curl_global_cleanup();
  return s.ptr;
}

const char* httpsPOST(const char* url, const char* data) {
  CURL *curl;
  CURLcode res;

  struct string pooh;

  pooh.ptr = data;
  pooh.len = (long)strlen(data);

  res = curl_global_init(CURL_GLOBAL_ALL);
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n",
        curl_easy_strerror(res));
    exit(EXIT_FAILURE);
  }

  curl = curl_easy_init();
  struct string s;
  if(curl) {
    init_string(&s);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    //curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    //curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pooh.ptr);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pooh.len);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pooh.len);
   
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(curl, CURLOPT_CAPATH, config.cert_path);
 

#ifdef DISABLE_EXPECT
    {
      struct curl_slist *chunk = NULL;
      chunk = curl_slist_append(chunk, "Expect:");
      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    }
#endif

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
          curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    printf("%s\n\n",s);
  }
  curl_global_cleanup();
  return s.ptr;
}



