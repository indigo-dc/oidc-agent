#ifndef OIDC_REFRESH_H
#define OIDC_REFRESH_H

#include "account/account.h"
#include "ipc/pipe.h"

char* refreshFlow(unsigned char return_mode, struct oidc_account* p,
                  const char* scope, const char* audience,
                  struct ipcPipe pipes);

#endif  // OIDC_REFRESH_H
