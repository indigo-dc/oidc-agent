#include "device.h"

#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/oidc/parse_oidp.h"
#include "oidc.h"
#include "utils/errorUtils.h"
#include "utils/logger.h"

char* generateDeviceCodePostData(const struct oidc_account* a) {
  return generatePostData(OIDC_KEY_CLIENTID, account_getClientId(a),
                          OIDC_KEY_SCOPE, account_getScope(a), NULL);
}

char* generateDeviceCodeLookupPostData(const struct oidc_account* a,
                                       const char*                device_code) {
  return generatePostData(
      // OIDC_KEY_CLIENTID, account_getClientId(a),
      // OIDC_KEY_CLIENTSECRET, account_getClientSecret(a),
      OIDC_KEY_GRANTTYPE, OIDC_GRANTTYPE_DEVICE, OIDC_KEY_DEVICECODE,
      device_code,
      // OIDC_KEY_RESPONSETYPE, OIDC_RESPONSETYPE_TOKEN,
      NULL);
}

struct oidc_device_code* initDeviceFlow(struct oidc_account* account) {
  logger(DEBUG, "Init device flow");
  const char* device_authorization_endpoint =
      account_getDeviceAuthorizationEndpoint(account);
  if (!strValid(device_authorization_endpoint)) {
    oidc_errno = OIDC_ENODEVICE;
    return NULL;
  }
  char* data = generateDeviceCodePostData(account);
  if (data == NULL) {
    return NULL;
  }
  logger(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      device_authorization_endpoint, data, account_getCertPath(account),
      account_getClientId(account), account_getClientSecret(account));
  secFree(data);
  if (res == NULL) {
    return NULL;
  }
  struct oidc_device_code* ret = parseDeviceCode(res);
  secFree(res);
  return ret;
}

void handleDeviceLookupError(const char* error, const char* error_description) {
  if (strequal(error, OIDC_SLOW_DOWN) ||
      strequal(error, OIDC_AUTHORIZATION_PENDING)) {
    oidc_seterror((char*)error);
  } else {
    if (error_description) {
      char* err = combineError(error, error_description);
      oidc_seterror(err);
      secFree(err);
    } else {
      oidc_seterror((char*)error);
    }
  }
  oidc_errno = OIDC_EOIDC;
}

oidc_error_t lookUpDeviceCode(struct oidc_account* account,
                              const char* device_code, struct ipcPipe pipes) {
  logger(DEBUG, "Doing Device Code Lookup\n");

  char* data = generateDeviceCodeLookupPostData(account, device_code);
  if (data == NULL) {
    return oidc_errno;
  }
  logger(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(account), data, account_getCertPath(account),
      account_getClientId(account), account_getClientSecret(account));
  secFree(data);
  if (res == NULL) {
    return oidc_errno;
  }

  char* access_token = parseTokenResponseCallbacks(
      res, account, 1, &handleDeviceLookupError, pipes);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}
