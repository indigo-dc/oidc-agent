#ifndef START_HTTPSERVER_H
#define START_HTTPSERVER_H

#include "defines/settings.h"
#include "list/list.h"
#include "utils/oidc_error.h"

oidc_error_t fireHttpServer(list_t* redirect_uris, size_t size,
                            char** state_ptr);

#endif  // START_HTTPSERVER_H
