#ifndef OIDC_AGENT_SUBMYTOKEN_H
#define OIDC_AGENT_SUBMYTOKEN_H

#include "account/account.h"
#include "ipc/pipe.h"

char* get_submytoken(struct ipcPipe pipes, struct oidc_account* account,
                     const char* profile, const char* application_hint);

#endif  // OIDC_AGENT_SUBMYTOKEN_H
