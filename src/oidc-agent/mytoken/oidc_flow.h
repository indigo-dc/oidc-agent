#ifndef OIDC_AGENT_MYTOKEN_OIDC_FLOW_H
#define OIDC_AGENT_MYTOKEN_OIDC_FLOW_H

#include "account/account.h"
#include "ipc/pipe.h"
#include "utils/oidc_error.h"

struct oidc_device_code* initMytokenOIDCFlow(struct oidc_account* account);
oidc_error_t             lookUpMytokenPollingCode(struct oidc_account* account,
                                                  const char*          polling_code,
                                                  struct ipcPipe       pipes);

#endif  // OIDC_AGENT_MYTOKEN_OIDC_FLOW_H
