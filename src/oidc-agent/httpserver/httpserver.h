#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "list/src/list.h"
#include "utils/oidc_error.h"

#include <microhttpd.h>

#define HTTP_DEFAULT_PORT 2912
#define HTTP_FALLBACK_PORT 8080

oidc_error_t fireHttpServer(list_t* redirect_uris, size_t size, char* config,
                            char* state);
void         termHttpServer(const char* state);

#endif  // HTTPSERVER_H
