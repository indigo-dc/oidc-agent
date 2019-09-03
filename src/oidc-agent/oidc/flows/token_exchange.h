#ifndef OIDC_TOKEN_EXCHANGE
#define OIDC_TOKEN_EXCHANGE

#include "account/account.h"
#include "ipc/pipe.h"
#include "utils/oidc_error.h"

oidc_error_t tokenExchange(struct oidc_account* account, struct ipcPipe pipes);

#endif  // OIDC_TOKEN_EXCHANGE
