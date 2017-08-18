#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include <curl/curl.h>

#include "http.h"
#include "oidc_error.h"

struct string {
  char *ptr;
  size_t len;
};

oidc_error_t init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);

  if(s->ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) malloc() failed: %m\n", __func__, __FILE__, __LINE__);
    oidc_errno = OIDC_EALLOC;
    return OIDC_EALLOC;
  }
  s->ptr[0] = '\0';
  return OIDC_SUCCESS;
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);

  if(s->ptr == NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
    oidc_errno = OIDC_EALLOC;
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

oidc_error_t CURLErrorHandling(int res, CURL* curl) {
  switch(res) {
    case CURLE_OK:
      return 0;
    case CURLE_URL_MALFORMAT:
    case CURLE_COULDNT_RESOLVE_HOST:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) HTTPS Request failed: %s Please check the provided URLs.\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      oidc_errno = OIDC_EURL;
      return OIDC_EURL;
    case CURLE_SSL_CONNECT_ERROR:
    case CURLE_SSL_CERTPROBLEM:
    case CURLE_SSL_CIPHER:
    case CURLE_SSL_CACERT:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_SSL_ISSUER_ERROR:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) HTTPS Request failed: %s Please check the provided certh_path.\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      oidc_errno = OIDC_ESSL;
      return OIDC_ESSL;
    default:
      syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) curl_easy_perform() failed: %s\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
      curl_easy_cleanup(curl);  
      return OIDC_EERROR;
  }
}

/** @fn CURL* init()
 * @brief initializes curl
 * @return a CURL pointer
 */
CURL* init() {
  CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
  if(CURLErrorHandling(res, NULL)!=OIDC_SUCCESS) {
    return NULL;
  }

  CURL* curl =  curl_easy_init();
  if(!curl) {
    curl_global_cleanup();
    syslog(LOG_AUTHPRIV|LOG_ALERT, "%s (%s:%d) Couldn't init curl. %s\n", __func__, __FILE__, __LINE__,  curl_easy_strerror(res));
    oidc_errno = OIDC_ECURLI;
    return NULL;
  }
  return curl;
}

/** @fn void setSSLOpts(CURL* curl)
 * @brief sets SSL options
 * @param curl the curl instance
 */
void setSSLOpts(CURL* curl, const char* cert_file) {
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
  if(cert_file) {
    char ca[strlen(cert_file)+1];
    strcpy(ca, cert_file);
    ca[strlen(cert_file)] = '\0';
    curl_easy_setopt(curl, CURLOPT_CAINFO, ca); 
  }
}

/** @fn void setWritefunction(CURL* curl, struct string* s)
 * @brief sets the function and buffer for writing the response
 * @param curl the curl instance
 * @param s the string struct where the response will be stored
 */
oidc_error_t setWriteFunction(CURL* curl, struct string* s) {
  oidc_error_t e;
  if((e = init_string(s))!=OIDC_SUCCESS) {
    return e;
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
  return OIDC_SUCCESS;
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
oidc_error_t perform(CURL* curl) {
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
  CURL* curl = init();
  setUrl(curl, url);
  struct string s;
  if(setWriteFunction(curl, &s)!=OIDC_SUCCESS) {
    return NULL;
  }
  setSSLOpts(curl, cert_path);
  if(perform(curl)!=OIDC_SUCCESS) {
    return NULL;
  }
  cleanup(curl);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
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
  CURL* curl = init();
  setUrl(curl, url);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  struct string s;
  if(setWriteFunction(curl, &s)!=OIDC_SUCCESS) {
    return NULL;
  }
  setPostData(curl, data);
  setSSLOpts(curl, cert_path);
  if(perform(curl)!=OIDC_SUCCESS) {
    return NULL;
  }
  cleanup(curl);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Response: %s\n",s.ptr);
  return s.ptr;
}

