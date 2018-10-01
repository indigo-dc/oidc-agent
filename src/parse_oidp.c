#include "json.h"
#include "account.h"
#include "device_code.h"
#include "utils/stringUtils.h"
#include "utils/errorUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

char* parseForError(char* res) {
  struct key_value pairs[2];
  pairs[0].key = "error";
  pairs[1].key = "error_description";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    return NULL;
  }
  clearFreeString(res);

  if(pairs[1].value) {
    char* error = combineError(pairs[0].value, pairs[1].value);
    clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
    return error;
  }
  return pairs[0].value;
}

struct oidc_device_code* parseDeviceCode(char* res) {
  if(!isJSONObject(res)) {
    return NULL;
  }
  char* copy = oidc_strcopy(res);
  char* error = parseForError(copy);
  if(error) {
    oidc_seterror(error);
    oidc_errno = OIDC_EOIDC;
    clearFreeString(error);
    return NULL;
  }
  return getDeviceCodeFromJSON(res);  
}

oidc_error_t parseOpenidConfiguration(char* res, struct oidc_account* account) {
 struct key_value pairs[8];
  pairs[0].key = "token_endpoint"; pairs[0].value = NULL;
  pairs[1].key = "authorization_endpoint"; pairs[1].value = NULL;
  pairs[2].key = "registration_endpoint"; pairs[2].value = NULL;
  pairs[3].key = "revocation_endpoint"; pairs[3].value = NULL;
  pairs[4].key = "device_authorization_endpoint"; pairs[3].value = NULL;
  pairs[5].key = "scopes_supported"; pairs[4].value = NULL;
  pairs[6].key = "grant_types_supported"; pairs[5].value = NULL;
  pairs[7].key = "response_types_supported"; pairs[6].value = NULL;
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    clearFreeString(res);
    return oidc_errno;
  }
  clearFreeString(res);

  if(pairs[0].value==NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Could not get token_endpoint");
    clearFreeKeyValuePairs(pairs, sizeof(pairs)/sizeof(*pairs));
    oidc_seterror("Could not get token_endpoint from the configuration_endpoint. This could be because of a network issue. But it's more likely that your issuer is not correct.");
    oidc_errno = OIDC_EERROR;
    return oidc_errno;
  }
  if(pairs[0].value) {issuer_setTokenEndpoint(account_getIssuer(*account), pairs[0].value);}
  if(pairs[1].value) {issuer_setAuthorizationEndpoint(account_getIssuer(*account), pairs[1].value);}
  if(pairs[2].value) {issuer_setRegistrationEndpoint(account_getIssuer(*account), pairs[2].value);}
  if(pairs[3].value) {issuer_setRevocationEndpoint(account_getIssuer(*account), pairs[3].value);}
  if(pairs[4].value) {issuer_setDeviceAuthorizationEndpoint(account_getIssuer(*account), pairs[4].value);}
  if(pairs[6].value==NULL) {
    const char* defaultValue = "[\"authorization_code\", \"implicit\"]";
    pairs[6].value = oidc_sprintf("%s", defaultValue);
  }
  char* scopes_supported = JSONArrrayToDelimitedString(pairs[5].value, ' ');
  if(scopes_supported==NULL) {
    clearFreeString(pairs[5].value);
    clearFreeString(pairs[6].value);
    clearFreeString(pairs[7].value);
    return oidc_errno;
  }
  account_setScopesSupported(account, scopes_supported);    
  clearFreeString(pairs[5].value);
  issuer_setGrantTypesSupported(account_getIssuer(*account), pairs[6].value);
  issuer_setResponseTypesSupported(account_getIssuer(*account), pairs[7].value);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Successfully retrieved endpoints.");
  return OIDC_SUCCESS;
}
