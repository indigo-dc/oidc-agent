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
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) malloc() failed: %m\n", __func__, __FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);

  if (s->ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
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
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) HTTPS Request failed: %s Please check the provided URLs.\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      return 1;
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_SSL_CACERT:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_SSL_ISSUER_ERROR:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) HTTPS Request failed: %s Please check the provided certh_path.\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      return 2;
    default:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) curl_easy_perform() failed: %s\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      return -1;
  }
}

/** @fn CURL* init()
 * @brief initializes curl
 * @return a CURL pointer
 */
CURL* init() {
  CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
  CURLErrorHandling(res, NULL);

  CURL* curl =  curl_easy_init();
  if(!curl) {
    curl_global_cleanup();
    syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) Couldn't init curl. %s\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
    exit(EXIT_FAILURE);
  }
  return curl;
}

/** @fn void setSSLOpts(CURL* curl)
 * @brief sets SSL options
 * @param curl the curl instance
 */
void setSSLOpts(CURL* curl, const char* cert_path) {
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
  if(cert_path) {
    char ca[strlen(cert_path)+1];
    strcpy(ca, cert_path);
    ca[strlen(cert_path)] = '\0';
    curl_easy_setopt(curl, CURLOPT_CAPATH, ca); 
  }
}

/** @fn void setWritefunction(CURL* curl, struct string* s)
 * @brief sets the function and buffer for writing the response
 * @param curl the curl instance
 * @param s the string struct where the response will be stored
 */
void setWriteFunction(CURL* curl, struct string* s) {
  init_string(s);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
}

/** @fn void setUrl(CURL* curl, const char* url)
 * @brief sets the url
 * @param curl the curl instance
 * @param url the url
 */
void setUrl(CURL* curl, const char* url) {
  curl_easy_setopt(curl, CURLOPT_URL, url);
}

/** @fn void setPostData(CURL* curl, const char* data)
 * @brief sets the data to be posted
 * @param curl the curl instance
 * @param data the data to be posted
 */
void setPostData(CURL* curl, const char* data) {
  long data_len = (long)strlen(data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
}

/** @fn int perform(CURL* curl)
 * @brief performs the https request and checks for errors
 * @param curl the curl instance
 * @return 0 on success, for error values see \f CURLErrorHandling
 */
int perform(CURL* curl) {
  CURLcode res = curl_easy_perform(curl);
  return CURLErrorHandling(res, curl);
}

/** @fn void cleanup(CURL* curl)
 * @brief does a easy and global cleanup
 * @param curl the curl instance
 */
void cleanup(CURL* curl) {
  curl_easy_cleanup(curl);  
  curl_global_cleanup();
}

/** @fn char* httpsGET(const char* url, const char* cert_path)
 * @brief does a https GET request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* httpsGET(const char* url, const char* cert_path) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Https GET to: %s",url);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "CA_PATH is %s", cert_path);
  CURL* curl = init();
  setUrl(curl, url);
  struct string s;
  setWriteFunction(curl, &s);
  setSSLOpts(curl, cert_path);
  if(perform(curl)!=0)
    return NULL;
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "CA_PATH is %s", cert_path);
  cleanup(curl);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "CA_PATH is %s", cert_path);
  return s.ptr;
}


/** @fn char* httpsPOST(const char* url, const char* data, const char* cert_path)
 * @brief does a https POST request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @param data the data to be posted
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* httpsPOST(const char* url, const char* data, const char* cert_path) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Https POST to: %s",url);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "CA_PATH is %s", cert_path);
  CURL* curl = init();
  setUrl(curl, url);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  struct string s;
  setWriteFunction(curl, &s);
  setPostData(curl, data);
  setSSLOpts(curl, cert_path);
  if(perform(curl)!=0)
    return NULL;
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "CA_PATH is %s", cert_path);
  cleanup(curl);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "CA_PATH is %s", cert_path);
  return s.ptr;
}

