#include "device.h"

#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/oidc/parse_oidp.h"
#include "oidc-agent/oidcd/deviceCodeEntry.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/db/deviceCode_db.h"
#include "utils/errorUtils.h"
#include "utils/string/stringUtils.h"

char* generateDeviceCodePostData(const struct oidc_account* a) {
  return generatePostData(OIDC_KEY_CLIENTID, account_getClientId(a),
                          OIDC_KEY_SCOPE, account_getScope(a), NULL);
}

char* generateDeviceCodeLookupPostData(const struct oidc_account* a,
                                       const char*                device_code) {
  char*   tmp_devicecode = oidc_strcopy(device_code);
  list_t* postDataList   = list_new();
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTID));
  // list_rpush(postDataList, list_node_new(account_getClientId(a)));
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTSECRET));
  // list_rpush(postDataList, list_node_new(account_getClientSecret(a)));
  list_rpush(postDataList, list_node_new(OIDC_KEY_GRANTTYPE));
  list_rpush(postDataList, list_node_new(OIDC_GRANTTYPE_DEVICE));
  list_rpush(postDataList, list_node_new(OIDC_KEY_DEVICECODE));
  list_rpush(postDataList, list_node_new(tmp_devicecode));
  if (strValid(account_getAudience(a))) {
    list_rpush(postDataList, list_node_new(OIDC_KEY_AUDIENCE));
    list_rpush(postDataList, list_node_new(account_getAudience(a)));
  }
  char* str = generatePostDataFromList(postDataList);
  list_destroy(postDataList);
  secFree(tmp_devicecode);
  return str;
}

struct oidc_device_code* initDeviceFlow(struct oidc_account* account) {
  agent_log(DEBUG, "Init device flow");
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
  agent_log(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      device_authorization_endpoint, data, account_getCertPath(account),
      account_getClientId(account), account_getClientSecret(account));
  secFree(data);
  if (res == NULL) {
    return NULL;
  }
  struct oidc_device_code* deviceCode = parseDeviceCode(res);
  secFree(res);
  if (deviceCode != NULL) {
    deviceCodeDB_addValue(
        createDeviceCodeEntry(deviceCode->device_code, account));
  }
  return deviceCode;
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
  agent_log(DEBUG, "Doing Device Code Lookup\n");

  char* data = generateDeviceCodeLookupPostData(account, device_code);
  if (data == NULL) {
    return oidc_errno;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(account), data, account_getCertPath(account),
      account_getClientId(account), account_getClientSecret(account));
  secFree(data);
  if (res == NULL) {
    return oidc_errno;
  }

  char* access_token = parseTokenResponseCallbacks(
      TOKENPARSEMODE_RETURN_AT | TOKENPARSEMODE_SAVE_AT, res, account,
      &handleDeviceLookupError, pipes, 0);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}
