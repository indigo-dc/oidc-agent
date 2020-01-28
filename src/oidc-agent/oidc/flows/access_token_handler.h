#ifndef FLOW_HANDLER_H
#define FLOW_HANDLER_H

#include "account/account.h"
#include "ipc/pipe.h"
#include "list/list.h"
#include "utils/oidc_error.h"

#include <time.h>

#define FORCE_NEW_TOKEN -1

char*        getAccessTokenUsingRefreshFlow(struct oidc_account* account,
                                            time_t min_valid_period, const char* scope,
                                            const char*    audience,
                                            struct ipcPipe pipes);
char*        getIdToken(struct oidc_account* p, const char* scope,
                        struct ipcPipe pipes);
oidc_error_t getAccessTokenUsingPasswordFlow(struct oidc_account* account,
                                             struct ipcPipe       pipes);
oidc_error_t getAccessTokenUsingAuthCodeFlow(struct oidc_account* account,
                                             const char*          code,
                                             const char*    used_redirect_uri,
                                             char*          code_verifier,
                                             struct ipcPipe pipes);
oidc_error_t getAccessTokenUsingDeviceFlow(struct oidc_account* account,
                                           const char*          device_code,
                                           struct ipcPipe       pipes);

list_t* parseFlow(const char* flow);
#endif  // FLOW_HANDLER_H
