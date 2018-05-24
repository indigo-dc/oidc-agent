#include "http_handler.h"

#include "../oidc_string.h"
#include "http_errorHandler.h"

#include <stdlib.h>
#include <syslog.h>

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
    curl_easy_setopt(curl, CURLOPT_CAINFO, cert_file); 
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

/** @fn void setUrlEncodedData(CURL* curl, const char* data)
 * @brief sets the data to be posted
 * @param curl the curl instance
 * @param data the data to be posted
 */
void setUrlEncodedData(CURL* curl, const char* data) {
  long data_len = (long)strlen(data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_len);
}

void setHeaders(CURL* curl, struct curl_slist* headers) {
  if(headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }
}

void setBasicAuth(CURL* curl, const char* username, const char* password) {
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(curl, CURLOPT_USERNAME, username);
  curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
}

/** @fn int perform(CURL* curl)
 * @brief performs the https request and checks for errors
 * @param curl the curl instance
 * @return 0 on success, for error values see \f CURLErrorHandling
 */
oidc_error_t perform(CURL* curl) {
  // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  // curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
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
