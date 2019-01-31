#include "parse_oidp.h"
#include "account/account.h"
#include "defines/oidc_values.h"
#include "device_code.h"
#include "utils/errorUtils.h"
#include "utils/json.h"
#include "utils/stringUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

char* parseForError(char* res) {
  struct key_value pairs[2];
  pairs[0].key = OIDC_KEY_ERROR;
  pairs[1].key = OIDC_KEY_ERROR_DESCRIPTION;
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    return NULL;
  }
  secFree(res);

  if (pairs[1].value) {
    char* error = combineError(pairs[0].value, pairs[1].value);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    return error;
  }
  return pairs[0].value;
}

struct oidc_device_code* parseDeviceCode(const char* res) {
  if (!isJSONObject(res)) {
    return NULL;
  }
  char* copy  = oidc_strcopy(res);
  char* error = parseForError(copy);
  if (error) {
    oidc_seterror(error);
    oidc_errno = OIDC_EOIDC;
    secFree(error);
    return NULL;
  }
  return getDeviceCodeFromJSON(res);
}

oidc_error_t parseOpenidConfiguration(char* res, struct oidc_account* account) {
  size_t           len = 9;
  struct key_value pairs[len];
  for (size_t i = 0; i < len; i++) { pairs[i].value = NULL; }
  pairs[0].key = OIDC_KEY_TOKEN_ENDPOINT;
  pairs[1].key = OIDC_KEY_AUTHORIZATION_ENDPOINT;
  pairs[2].key = OIDC_KEY_REGISTRATION_ENDPOINT;
  pairs[3].key = OIDC_KEY_REVOCATION_ENDPOINT;
  pairs[4].key = OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT;
  pairs[5].key = OIDC_KEY_SCOPES_SUPPORTED;
  pairs[6].key = OIDC_KEY_GRANT_TYPES_SUPPORTED;
  pairs[7].key = OIDC_KEY_RESPONSE_TYPES_SUPPORTED;
  pairs[8].key = OIDC_KEY_CODE_CHALLENGE_METHODS_SUPPORTED;
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    secFree(res);
    return oidc_errno;
  }
  secFree(res);

  if (pairs[0].value == NULL) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Could not get token endpoint");
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    oidc_seterror(
        "Could not get token endpoint from the configuration endpoint. This "
        "could be because of a network issue. But it's more likely that your "
        "issuer is not correct.");
    oidc_errno = OIDC_EERROR;
    return oidc_errno;
  }
  struct oidc_issuer* issuer = account_getIssuer(account);
  if (pairs[0].value) {
    issuer_setTokenEndpoint(issuer, pairs[0].value);
  }
  if (pairs[1].value) {
    issuer_setAuthorizationEndpoint(issuer, pairs[1].value);
  }
  if (pairs[2].value) {
    issuer_setRegistrationEndpoint(issuer, pairs[2].value);
  }
  if (pairs[3].value) {
    issuer_setRevocationEndpoint(issuer, pairs[3].value);
  }
  if (pairs[4].value) {
    issuer_setDeviceAuthorizationEndpoint(issuer, pairs[4].value, 0);
  }
  if (pairs[6].value == NULL) {
    const char* defaultValue = OIDC_PROVIDER_DEFAULT_GRANTTYPES;
    pairs[6].value           = oidc_sprintf("%s", defaultValue);
  }
  char* scopes_supported =
      JSONArrayStringToDelimitedString(pairs[5].value, ' ');
  if (scopes_supported == NULL) {
    secFree(pairs[5].value);
    secFree(pairs[6].value);
    secFree(pairs[7].value);
    secFree(pairs[8].value);
    return oidc_errno;
  }
  account_setScopesSupported(account, scopes_supported);
  secFree(pairs[5].value);
  issuer_setGrantTypesSupported(account_getIssuer(account), pairs[6].value);
  issuer_setResponseTypesSupported(account_getIssuer(account), pairs[7].value);
  if (pairs[8].value) {
    if (strSubString(pairs[8].value, CODE_CHALLENGE_METHOD_S256)) {
      account_setCodeChallengeMethod(account, CODE_CHALLENGE_METHOD_S256);
    } else if (strSubString(pairs[8].value, CODE_CHALLENGE_METHOD_PLAIN)) {
      account_setCodeChallengeMethod(account, CODE_CHALLENGE_METHOD_PLAIN);
    }
    secFree(pairs[8].value);
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Successfully retrieved endpoints.");
  return OIDC_SUCCESS;
}
