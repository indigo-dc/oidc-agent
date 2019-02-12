#ifndef OIDCGEN_PROMPTANDSET_H
#define OIDCGEN_PROMPTANDSET_H

#include "account/account.h"
#include "list/list.h"
#include "oidc-gen/oidc-gen_options.h"

void promptAndSetIssuer(struct oidc_account* account);
void promptAndSetClientId(struct oidc_account* account);
void promptAndSetClientSecret(struct oidc_account* account, int);
void promptAndSetRefreshToken(struct oidc_account* account,
                              struct optional_arg  refresh_token);
void promptAndSetUsername(struct oidc_account* account, list_t* flows);
void promptAndSetPassword(struct oidc_account* account, list_t* flows);
void promptAndSetCertPath(struct oidc_account* account,
                          struct optional_arg  cert_path);
void promptAndSetName(struct oidc_account* account, const char* short_name,
                      const char* client_name_id);
void promptAndSetScope(struct oidc_account* account);
void promptAndSetRedirectUris(struct oidc_account* account, int useDevice);

#endif  // OIDCGEN_PROMPTANDSET_H
