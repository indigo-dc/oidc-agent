#ifndef OIDC_DELETE_CLIENT_H
#define OIDC_DELETE_CLIENT_H

#include "utils/oidc_error.h"

oidc_error_t deleteClient(const char* configuration_endpoint,
                          const char* registration_access_token,
                          const char* cert_path);

#endif  // OIDC_DELETE_CLIENT_H
