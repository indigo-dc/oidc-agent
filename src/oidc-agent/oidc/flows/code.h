#ifndef OIDC_CODE_H
#define OIDC_CODE_H

#include "account/account.h"
#include "ipc/pipe.h"
#include "utils/oidc_error.h"

char* buildCodeFlowUri(const struct oidc_account* account, char** state_ptr,
                       const char* code_verifier);
oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri, char* code_verifier,
                          struct ipcPipe pipes);

#endif  // OIDC_CODE_H
