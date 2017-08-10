#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include <curl/curl.h>

#include "http.h"

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);

  if (s->ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "malloc() failed int function init_string() %m\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);

  if (s->ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "realloc() failed int function write_callback() %m\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

int CURLErrorHandling(int res, CURL* curl) {
  switch(res) {
    case CURLE_OK:
      return 0;
    case CURLE_URL_MALFORMAT:
    case CURLE_COULDNT_RESOLVE_HOST:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "HTTPS Request failed: %s Please check the provided URLs.\n",  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      return 1;
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_SSL_CACERT:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_SSL_ISSUER_ERROR:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "HTTPS Request failed: %s Please check the provided certh_path.\n",  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      return 2;
    default:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "curl_easy_perform() failed: %s\n",  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      return -1;
  }
}

CURL* init() {
  CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
  CURLErrorHandling(res, NULL);

  CURL* curl =  curl_easy_init();
  if(!curl) {
    curl_global_cleanup();
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Couldn't init curl. %s\n",  curl_easy_strerror(res));
    exit(EXIT_FAILURE);
  }
  return curl;
}

void setSSLOpts(CURL* curl) {
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
  curl_easy_setopt(curl, CURLOPT_CAPATH, "/etc/ssl/certs"); //TODO FIXME XXX
}

void setWriteFunction(CURL* curl, struct string* s) {
  init_string(s);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
}

void setUrl(CURL* curl, const char* url) {
  curl_easy_setopt(curl, CURLOPT_URL, url);
}

void setPostData(CURL* curl, const char* data) {
  long data_len = (long)strlen(data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
}

int perform(CURL* curl) {
  CURLcode res = curl_easy_perform(curl);
  return CURLErrorHandling(res, curl);
}

void cleanup(CURL* curl) {
  curl_easy_cleanup(curl);  
  curl_global_cleanup();
}

/** @fn char* httpsGET(const char* url)
 * @brief does a https GET request
 * @param url the request url
 * @return a pointer to the response. Has to be freed after usage.
 */
char* httpsGET(const char* url) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Https GET to: %s",url);
  CURL* curl = init();
  setUrl(curl, url);
  struct string s;
  setWriteFunction(curl, &s);
  setSSLOpts(curl);
  if(perform(curl)!=0)
    return NULL;
  cleanup(curl);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
  return s.ptr;
}


/** @fn char* httpsPOST(const char* url, const char* data)
 * @brief does a https POST request
 * @param url the request url
 * @param data the data to be posted
 * @return a pointer to the response. Has to be freed after usage.
 */
char* httpsPOST(const char* url, const char* data) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Https POST to: %s",url);

  CURL* curl = init();
  setUrl(curl, url);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  struct string s;
  setWriteFunction(curl, &s);
  setPostData(curl, data);
  setSSLOpts(curl);
  if(perform(curl)!=0)
    return NULL;
  cleanup(curl);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
  return s.ptr;
}

