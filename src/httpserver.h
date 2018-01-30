#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "oidc_error.h"

#include <microhttpd.h>

#define HTTP_DEFAULT_PORT 2912
#define HTTP_FALLBACK_PORT 8080

oidc_error_t fireHttpServer(unsigned short port, char* config, char* state) ;
void termHttpServer(char* state);

#endif // HTTPSERVER_H
