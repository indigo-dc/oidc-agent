#ifndef OIDC_CODE_H
#define OIDC_CODE_H

#include "account/account.h"
#include "ipc/pipe.h"
#include "utils/oidc_error.h"

char* buildCodeFlowUri(const struct oidc_account* account, char** state_ptr,
                       char** code_verifier_ptr, const unsigned char only_at);
oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri, char* code_verifier,
                          struct ipcPipe pipes);

char* _removeScope(char* scopes, char* rem);

#endif  // OIDC_CODE_H
