#ifndef OIDC_PROXY_HANDLER_H
#define OIDC_PROXY_HANDLER_H

#include "utils/oidc_error.h"

oidc_error_t updateRefreshToken(const char* shortname,
                                const char* refresh_token);
#endif  // OIDC_PROXY_HANDLER_H
