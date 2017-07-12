#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "http.h"
#include "config.h"
#include "logger.h"



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

void CURLErrorHandling(int res, CURL* curl) {
  const char* url = NULL;
  curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
  switch(res) {
    case CURLE_OK:
      return;
    case CURLE_URL_MALFORMAT:
    case CURLE_COULDNT_RESOLVE_HOST:
      fprintf(stderr, "HTTP Request to '%s' failed: %s\nPlease check the provided URLs.\n", url, curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      exit(EXIT_FAILURE);
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_SSL_CACERT:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_SSL_ISSUER_ERROR:
      fprintf(stderr, "HTTP Request to '%s' failed due to a SSL ERROR: %s\nPlease check your certh_path.\n", url, curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      exit(EXIT_FAILURE);
    default:
      fprintf(stderr, "curl_easy_perform() failed on request to '%s': %s\n", url, curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      exit(EXIT_FAILURE); 
  }
}

const char* httpsGET(const char* url) {
  logging(3, "Https GET to: %s",url);
  CURL* curl;
  CURLcode res;

  res = curl_global_init(CURL_GLOBAL_ALL);
    CURLErrorHandling(res, curl);
 
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
    CURLErrorHandling(res, curl);

    curl_easy_cleanup(curl);  
    logging(3, "%s\n",s);
  } 
  else {
    fprintf(stderr, "Couldn't init curl.\n");
    exit(EXIT_FAILURE);
  }
  curl_global_cleanup();
  return s.ptr;
}


const char* httpsPOST(const char* url, const char* data) {
  logging(3, "Https POST to: %s",url);
  CURL *curl;
  CURLcode res;

  long data_len = (long)strlen(data);

  res = curl_global_init(CURL_GLOBAL_ALL);
    CURLErrorHandling(res, curl);
 
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
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
   
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
    CURLErrorHandling(res, curl);

    curl_easy_cleanup(curl);
    logging(3, "%s\n",s);
  }
  curl_global_cleanup();
  return s.ptr;
}



