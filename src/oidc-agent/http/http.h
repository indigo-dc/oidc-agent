#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>

char* _httpsGET(const char* url, struct curl_slist* list,
                const char* cert_path);
char* _httpsPOST(const char* url, const char* data, struct curl_slist* headers,
                 const char* cert_path, const char* username,
                 const char* password);
char* _httpsDELETE(const char* url, struct curl_slist* headers,
                   const char* cert_path, const char* bearer_token);
#endif
