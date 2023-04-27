#include "http_handler.h"

#include <stdlib.h>
#include <string.h>

#include "defines/version.h"
#include "http_errorHandler.h"
#include "utils/agentLogger.h"
#include "utils/memory.h"
#include "utils/sleeper.h"
#include "utils/string/oidc_string.h"
#include "utils/string/stringUtils.h"

static size_t write_callback(void* ptr, size_t size, size_t nmemb,
                             struct string* s) {
  size_t new_len = s->len + size * nmemb;
  void*  tmp     = secRealloc(s->ptr, new_len + 1);
  if (tmp == NULL) {
    exit(EXIT_FAILURE);
  }
  s->ptr = tmp;
  memcpy(s->ptr + s->len, ptr, size * nmemb);
  // s->ptr[new_len] = '\0';
  s->len = new_len;

  return size * nmemb;
}

#ifndef AGENT_CURL_CONNECT_TIMEOUT
#define AGENT_CURL_CONNECT_TIMEOUT 5
#endif
#ifndef AGENT_CURL_TIMEOUT
#define AGENT_CURL_TIMEOUT 10
#endif

static unsigned char mem_init = 0;

void curlMemInit() {
  if (!mem_init) {
    CURLcode res = curl_global_init_mem(CURL_GLOBAL_ALL, secAlloc, _secFree,
                                        secRealloc, oidc_strcopy, secCalloc);
    if (CURLErrorHandling(res, NULL) != OIDC_SUCCESS) {
      return NULL;
    }
    mem_init = 1;
  }
}

/** @fn CURL* init()
 * @brief initializes curl
 * @return a CURL pointer
 */
CURL* init() {
  curlMemInit();

  CURL* curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    agent_log(ALERT, "%s (%s:%d) Couldn't init curl.\n", __func__, __FILE__,
              __LINE__);
    oidc_errno = OIDC_ECURLI;
    return NULL;
  }
  curl_easy_setopt(curl, CURLOPT_USERAGENT, AGENT_VERSION);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, AGENT_CURL_TIMEOUT);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, AGENT_CURL_CONNECT_TIMEOUT);
  return curl;
}

/** @fn void setSSLOpts(CURL* curl)
 * @brief sets SSL options
 * @param curl the curl instance
 */
void setSSLOpts(CURL* curl, const char* cert_file) {
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
  if (cert_file) {
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
  if ((e = init_string(s)) != OIDC_SUCCESS) {
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

void setHeaders(CURL* curl, struct curl_slist* headers) {
  if (headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }
}

void setBasicAuth(CURL* curl, const char* username, const char* password) {
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(curl, CURLOPT_USERNAME, username);
  curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
  // agent_log(DEBUG, "Http Set Client credentials: %s - %s",
  //        username ?: "NULL", password ?: "NULL");
}

// void setTokenAuth(CURL* curl, const char* token) {
//   curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER); // This is only
//   available since curl 7.61 curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER,
//   token);
// }

#ifndef AGENT_CURL_MAX_TRIES
#define AGENT_CURL_MAX_TRIES 3
#endif

/** @fn int perform(CURL* curl)
 * @brief performs the https request and checks for errors
 * @param curl the curl instance
 * @return 0 on success, for error values see \f CURLErrorHandling
 */
oidc_error_t perform(CURL* curl) {
  // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  // curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
  CURLcode     res;
  unsigned int tries   = 0;
  unsigned int sleep_t = 1;
  do {
    if (tries != 0) {
      msleep(sleep_t * 1000);
      sleep_t *= 2;
    }
    tries++;
    res = curl_easy_perform(curl);
    switch (res) {
      case CURLE_OPERATION_TIMEDOUT:
      case CURLE_COULDNT_CONNECT:
      case CURLE_COULDNT_RESOLVE_HOST:
      case CURLE_COULDNT_RESOLVE_PROXY: break;
      case CURLE_OK: {
        long status;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        switch (status) {
          case 408:  // Request Timeout
          case 429:  // Too Many Requests
          case 502:  // Bad Gateway
          case 503:  // Service Unavailable
          case 504:  // Gateway Timeout
            break;
          default: return CURLErrorHandling(res, curl);
        }
        break;
      }
      default: return CURLErrorHandling(res, curl);
    }
  } while (tries < AGENT_CURL_MAX_TRIES);

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
