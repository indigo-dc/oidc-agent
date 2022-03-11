#ifndef HTTP_IPC_H
#define HTTP_IPC_H

#include "http.h"

#define HTTP_HEADER_ACCEPT_JSON "Accept: application/json"
#define HTTP_HEADER_CONTENTTYPE_JSON "Content-Type: application/json"
#define HTTP_HEADER_AUTHORIZATION_BEARER_FMT "Authorization: Bearer %s"

char* httpsGET(const char* url, struct curl_slist* list, const char* cert_path);
char* httpsPOST(const char* url, const char* data, struct curl_slist* headers,
                const char* cert_path, const char* username,
                const char* password);
char* httpsDELETE(const char* url, struct curl_slist* headers,
                  const char* cert_path, const char* bearer_token);

char* sendPostDataWithBasicAuth(const char* endpoint, const char* data,
                                const char* cert_path, const char* username,
                                const char* password);
char* sendPostDataWithoutBasicAuth(const char* endpoint, const char* data,
                                   const char* cert_path);

#endif  // HTTP_IPC_H
