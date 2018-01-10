#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <microhttpd.h>

struct MHD_Daemon* startHttpServer(unsigned short port) ;
void stopHttpServer(struct MHD_Daemon* d) ;

#endif // HTTPSERVER_H
