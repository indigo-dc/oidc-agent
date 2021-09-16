#ifndef OIDC_DEVICE_H
#define OIDC_DEVICE_H

#include "account/account.h"
#include "ipc/pipe.h"
#include "oidc-agent/oidc/device_code.h"

struct oidc_device_code* initDeviceFlow(struct oidc_account* account);
oidc_error_t             lookUpDeviceCode(struct oidc_account* account,
                                          const char* device_code, struct ipcPipe pipes);

#endif  // OIDC_DEVICE_H
