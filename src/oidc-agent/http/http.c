#define _POSIX_C_SOURCE 200112L

#include "http.h"

#include <curl/curl.h>
#include <time.h>

#include "http_handler.h"
#include "utils/agentLogger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/pass.h"
#include "utils/string/stringUtils.h"

static char* _formatTraceTime() {
  char* s = secAlloc(sizeof(char) * 20);
  if (s == NULL) {
    return NULL;
  }
  time_t    now = time(NULL);
  struct tm t;
  localtime_r(&now, &t);
  strftime(s, 20, "%F %H:%M:%S", &t);
  return s;
}

static void _writeTraceHeader(FILE* trace_file, const char* method,
                              const char* url) {
  if (trace_file == NULL) {
    return;
  }
  char* time_str = _formatTraceTime();
  fprintf(trace_file, "\n=== %s HTTPS %s to %s ===\n", time_str ?: "?", method,
          url);
  secFree(time_str);
  fflush(trace_file);
}

static void _writeTraceFooter(FILE* trace_file, double total_time) {
  if (trace_file == NULL) {
    return;
  }
  fprintf(trace_file, "=== Request completed in %.3fs ===\n\n", total_time);
  fflush(trace_file);
}

/** @fn char* httpsGET(const char* url, const char* cert_path)
 * @brief does a https GET request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* _httpsGET(const char* url, struct curl_slist* headers,
                const char* cert_path) {
  agent_log(DEBUG, "Https GET to: %s", url);
  CURL* curl       = init();
  FILE* trace_file = enableHttpTrace(curl);
  _writeTraceHeader(trace_file, "GET", url);
  setUrl(curl, url);
  struct string s;
  if (setWriteFunction(curl, &s) != OIDC_SUCCESS) {
    if (trace_file) {
      fclose(trace_file);
    }
    return NULL;
  }
  setSSLOpts(curl, cert_path);
  setHeaders(curl, headers);
  oidc_error_t err        = perform(curl);
  double       total_time = 0;
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
  agent_log(DEBUG, "Request completed in %.3fs", total_time);
  _writeTraceFooter(trace_file, total_time);
  if (trace_file) {
    fclose(trace_file);
  }
  if (err != OIDC_SUCCESS) {
    if (err >= 200 && err < 600 && strValid(s.ptr)) {
      pass;
    } else {
      secFree(s.ptr);
      cleanup(curl);
      return NULL;
    }
  }
  cleanup(curl);
  agent_log(DEBUG, "Response: %s\n", s.ptr);
  return s.ptr;
}

/** @fn char* httpsDELETE(const char* url, const char* cert_path)
 * @brief does a https DELETE request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* _httpsDELETE(const char* url, struct curl_slist* headers,
                   const char* cert_path, const char* bearer_token) {
  agent_log(DEBUG, "Https DELETE to: %s", url);
  CURL* curl       = init();
  FILE* trace_file = enableHttpTrace(curl);
  _writeTraceHeader(trace_file, "DELETE", url);
  setUrl(curl, url);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  struct string s;
  if (setWriteFunction(curl, &s) != OIDC_SUCCESS) {
    if (trace_file) {
      fclose(trace_file);
    }
    return NULL;
  }
  setSSLOpts(curl, cert_path);
  char* bearer_header = oidc_sprintf("Authorization: Bearer %s", bearer_token);
  headers             = curl_slist_append(headers, bearer_header);
  setHeaders(curl, headers);
  // if (bearer_token) {
  //   setTokenAuth(curl, bearer_token);
  // }
  oidc_error_t err = perform(curl);
  secFree(bearer_header);
  double total_time = 0;
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
  agent_log(DEBUG, "Request completed in %.3fs", total_time);
  _writeTraceFooter(trace_file, total_time);
  if (trace_file) {
    fclose(trace_file);
  }
  if (err != OIDC_SUCCESS) {
    if (err >= 200 && err < 600 && strValid(s.ptr)) {
      pass;
    } else {
      secFree(s.ptr);
      cleanup(curl);
      return NULL;
    }
  }
  cleanup(curl);
  agent_log(DEBUG, "Response: %s\n", s.ptr);
  return s.ptr;
}

/** @fn char* httpsPOST(const char* url, const char* data, const char*
 * cert_path)
 * @brief does a https POST request
 * @param url the request url
 * @param cert_path the path to the SSL certs
 * @param data the data to be posted
 * @return a pointer to the response. Has to be freed after usage. If the Https
 * call failed, NULL is returned.
 */
char* _httpsPOST(const char* url, const char* data, struct curl_slist* headers,
                 const char* cert_path, const char* username,
                 const char* password) {
  agent_log(DEBUG, "Https POST to: %s", url);
  CURL* curl       = init();
  FILE* trace_file = enableHttpTrace(curl);
  _writeTraceHeader(trace_file, "POST", url);
  setUrl(curl, url);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  struct string s;
  if (setWriteFunction(curl, &s) != OIDC_SUCCESS) {
    if (trace_file) {
      fclose(trace_file);
    }
    return NULL;
  }
  setPostData(curl, data);
  setSSLOpts(curl, cert_path);
  setHeaders(curl, headers);
  if (username) {
    setBasicAuth(curl, username, password ?: "");
  }
  oidc_error_t err        = perform(curl);
  double       total_time = 0;
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
  agent_log(DEBUG, "Request completed in %.3fs", total_time);
  _writeTraceFooter(trace_file, total_time);
  if (trace_file) {
    fclose(trace_file);
  }
  if (err != OIDC_SUCCESS) {
    if (err >= 200 && err < 600 && strValid(s.ptr)) {
      pass;
    } else {
      secFree(s.ptr);
      cleanup(curl);
      return NULL;
    }
  }
  cleanup(curl);
  agent_log(DEBUG, "Response: %s\n", s.ptr ? s.ptr : "(null)");
  return s.ptr;
}
