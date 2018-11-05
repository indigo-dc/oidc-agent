#ifndef TERM_HTTPSERVER_H
#define TERM_HTTPSERVER_H

#include "list/src/list.h"

#include <microhttpd.h>
#include <stddef.h>

void termHttpServer(const char* state);
void stopHttpServer(struct MHD_Daemon** d_ptr);

static list_t* servers = NULL;

#endif  // TERM_HTTPSERVER_H
