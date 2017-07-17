#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <syslog.h>

#include "http.h"
#include "config.h"



void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);

  if (s->ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "malloc() failed int function init_string()\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);

  if (s->ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "realloc() failed int function write_callback()\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

void CURLErrorHandling(int res, CURL* curl) {
  switch(res) {
    case CURLE_OK:
      return;
    case CURLE_URL_MALFORMAT:
    case CURLE_COULDNT_RESOLVE_HOST:
      syslog(LOG_AUTHPRIV|LOG_EMERG, "HTTPS Request failed: %s Please check the provided URLs.\n",  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      exit(EXIT_FAILURE);
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_SSL_CACERT:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_SSL_ISSUER_ERROR:
      syslog(LOG_AUTHPRIV|LOG_EMERG, "HTTPS Request failed: %s Please check the provided certh_path.\n",  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      exit(EXIT_FAILURE);
    default:
      syslog(LOG_AUTHPRIV|LOG_EMERG, "curl_easy_perform() failed: %s\n",  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      exit(EXIT_FAILURE); 
  }
}

char* httpsGET(const char* url) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Https GET to: %s",url);
  CURL* curl;
  CURLcode res;

  res = curl_global_init(CURL_GLOBAL_ALL);
  CURLErrorHandling(res, NULL);

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
    curl_easy_setopt(curl, CURLOPT_CAPATH, conf_getCertPath());

    res = curl_easy_perform(curl);
    CURLErrorHandling(res, curl);

    curl_easy_cleanup(curl);  
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
  } 
  else {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Couldn't init curl for Https GET. %s\n",  curl_easy_strerror(res));
    exit(EXIT_FAILURE);
  }
  curl_global_cleanup();
  return s.ptr;
}


char* httpsPOST(const char* url, const char* data) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Https GET to: %s",url);
  CURL *curl;
  CURLcode res;

  long data_len = (long)strlen(data);

  res = curl_global_init(CURL_GLOBAL_ALL);
  CURLErrorHandling(res, NULL);

  curl = curl_easy_init();
  struct string s;
  if(curl) {
    init_string(&s);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(curl, CURLOPT_CAPATH, conf_getCertPath());


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
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
  } else {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Couldn't init curl for Https GET. %s\n",  curl_easy_strerror(res));
    exit(EXIT_FAILURE);
  }
  curl_global_cleanup();
  return s.ptr;
}



