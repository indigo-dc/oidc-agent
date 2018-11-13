#include "token_exchange.h"
#include "oidc.h"

char* generateDeviceCodeLookupPostData(struct oidc_account a) {
  return generatePostData(
      "grant_type", "urn:ietf:params:oauth:grant-type:token-exchange",
      "subject_token", account_getAccessToken(a), "subject_token_type",
      "urn:ietf:params:oauth:token-type:access_token", "requested_token_type",
      "urn:ietf:params:oauth:token-type:refresh_token", "scope",
      account_getScope(a), NULL);
}

oidc_error_t tokenExchange(struct oidc_account* account) {
  return OIDC_SUCCESS;
}
