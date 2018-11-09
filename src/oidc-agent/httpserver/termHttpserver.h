#ifndef TERM_HTTPSERVER_H
#define TERM_HTTPSERVER_H

#include "list/list.h"

#include <microhttpd.h>

void termHttpServer(const char* state);
void stopHttpServer(struct MHD_Daemon** d_ptr);

#endif  // TERM_HTTPSERVER_H
