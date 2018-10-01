#ifndef OIDC_DEVICE_H
#define OIDC_DEVICE_H

#include "../account.h"

struct oidc_device_code* initDeviceFlow(struct oidc_account* account) ;
oidc_error_t lookUpDeviceCode(struct oidc_account* account, const char* device_code) ;

#endif // OIDC_DEVICE_H
