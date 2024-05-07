#ifndef OIDC_CUSTOM_PARAMETER_H
#define OIDC_CUSTOM_PARAMETER_H

#include "account/account.h"
#include "wrapper/list.h"

#define OIDC_REQUEST_TYPE_REFRESH "refresh"
#define OIDC_REQUEST_TYPE_AUTHURL "auth_url"
#define OIDC_REQUEST_TYPE_CODEEXCHANGE "code-exchange"
#define OIDC_REQUEST_TYPE_DEVICEINIT "device-init"
#define OIDC_REQUEST_TYPE_DEVICEPOLLING "device-polling"
#define OIDC_REQUEST_TYPE_REVOKE "revocation"
#define OIDC_REQUEST_TYPE_PASSWORD "password"

void addCustomParameters(list_t*                    request_params,
                         const struct oidc_account* account,
                         const char*                request_type);

#endif  // OIDC_CUSTOM_PARAMETER_H
