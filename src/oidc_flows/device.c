#include "device.h"

#include "oidc.h"
#include "../http/http.h"
#include "../ipc/ipc_values.h"
#include "../parse_oidp.h"
#include "../utils/errorUtils.h"

#include <syslog.h>

char* generateDeviceCodePostData(struct oidc_account a) {
  return generatePostData(
      "client_id", account_getClientId(a),
      "scope", account_getScope(a),
      NULL);
}

char* generateDeviceCodeLookupPostData(struct oidc_account a, const char* device_code) {
  return generatePostData(
      "client_id", account_getClientId(a),
      "client_secret", account_getClientSecret(a),
      "grant_type", "urn:ietf:params:oauth:grant-type:device_code",
      "device_code", device_code,
      "response_type", "token",
      NULL);
}

struct oidc_device_code* initDeviceFlow(struct oidc_account* account) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Init device flow");
  const char* device_authorization_endpoint = account_getDeviceAuthorizationEndpoint(*account);
  if(!strValid(device_authorization_endpoint)) {
    oidc_errno = OIDC_ENODEVICE;
    return NULL;
  }
  char* data = generateDeviceCodePostData(*account);
  if(data==NULL) {
    return NULL;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = sendPostDataWithoutBasicAuth(device_authorization_endpoint, data, account_getCertPath(*account));
  clearFreeString(data);
  if(res==NULL) {
    return NULL;
  }
  return parseDeviceCode(res);
}

void handleDeviceLookupError(const char* error, const char* error_description) {
if(strcmp(error, OIDC_SLOW_DOWN)==0 || strcmp(error, OIDC_AUTHORIZATION_PENDING)==0) {
      oidc_seterror((char*) error);
    } else {
      if(error_description) {
        char* err = combineError(error, error_description);
          oidc_seterror(err);
          clearFreeString(err);
      } else {
      oidc_seterror((char*) error);
      }
    }
    oidc_errno = OIDC_EOIDC;
}

oidc_error_t lookUpDeviceCode(struct oidc_account* account, const char* device_code) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG,"Doing Device Code Lookup\n");

  char* data = generateDeviceCodeLookupPostData(*account, device_code);
  if(data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s",data);
  char* res = sendPostDataWithBasicAuth(account_getTokenEndpoint(*account), data, account_getCertPath(*account), account_getClientId(*account), account_getClientSecret(*account));
  clearFreeString(data);
  if(res==NULL) {
    return oidc_errno;
  }
  
  char* access_token = parseTokenResponseCallbacks(res, account, 1, 1, &handleDeviceLookupError);
  clearFreeString(res);
  return access_token==NULL ? oidc_errno : OIDC_SUCCESS;
}

