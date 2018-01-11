#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <microhttpd.h>

#define HTTP_DEFAULT_PORT 2912
#define HTTP_FALLBACK_PORT 8080

struct MHD_Daemon** startHttpServer(unsigned short port) ;
void stopHttpServer(struct MHD_Daemon** d) ;

#endif // HTTPSERVER_H
