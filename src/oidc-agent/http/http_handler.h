#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <curl/curl.h>

#include "utils/oidc_error.h"
#include "utils/oidc_string.h"

CURL*        init();
void         setSSLOpts(CURL* curl, const char* cert_file);
oidc_error_t setWriteFunction(CURL* curl, struct string* s);
void         setUrl(CURL* curl, const char* url);
void         setHeaders(CURL* curl, struct curl_slist* headers);
void setBasicAuth(CURL* curl, const char* username, const char* password);
oidc_error_t perform(CURL* curl);
void         cleanup(CURL* curl);

#endif  // HTTP_HANDLER_H
