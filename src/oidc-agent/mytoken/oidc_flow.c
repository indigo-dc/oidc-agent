#include "oidc_flow.h"

#include "account/account.h"
#include "defines/mytoken_values.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/oidc/flows/device.h"
#include "oidc-agent/oidc/flows/oidc.h"
#include "oidc-agent/oidc/parse_oidp.h"
#include "oidc-agent/oidcd/deviceCodeEntry.h"
#include "utils/agentLogger.h"
#include "utils/db/deviceCode_db.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

struct oidc_device_code* initMytokenOIDCFlow(struct oidc_account* account) {
  agent_log(DEBUG, "Init mytoken OIDC flow");
  const char* mytoken_endpoint = account_getMytokenEndpoint(account);
  if (!strValid(mytoken_endpoint)) {
    oidc_errno = OIDC_ENOMYTOKEN;
    return NULL;
  }
  char*  profile = account_getUsedMytokenProfile(account);
  cJSON* json    = profile ? cJSON_Parse(profile) : cJSON_CreateObject();
  if (json == NULL) {
    return NULL;
  }
  setJSONValue(json, "oidc_issuer", account_getIssuerUrl(account));
  setJSONValue(json, OIDC_KEY_GRANTTYPE, MYTOKEN_GRANTTYPE_OIDC);
  setJSONValue(json, "oidc_flow", OIDC_GRANTTYPE_AUTHCODE);
  setJSONValue(json, "client_type", "native");
  char* name = oidc_sprintf("mytoken for %s", account_getClientName(account));
  setJSONValueIfNotSet(json, "name", name);
  setJSONValue(json, OIDC_KEY_RESPONSETYPE, "token");
  char* data = jsonToStringUnformatted(json);
  secFreeJson(json);
  secFree(name);
  if (data == NULL) {
    return NULL;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* res = sendJSONPostWithoutBasicAuth(mytoken_endpoint, data,
                                           account_getCertPath(account), NULL);
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

void handleMytokenDeviceLookupError(const char* error,
                                    const char* error_description) {
  handleDeviceLookupError(error, error_description);
  oidc_errno = OIDC_EMYTOKEN;
}

char* generateMytokenPollingCodeLookupPostData(const char* polling_code) {
  return oidc_sprintf("{\"" OIDC_KEY_GRANTTYPE
                      "\":\"" MYTOKEN_GRANTTYPE_POLLINGCODE
                      "\",\"" MYTOKEN_KEY_POLLINGCODE "\":\"%s\"}",
                      polling_code);
}

oidc_error_t lookUpMytokenPollingCode(struct oidc_account* account,
                                      const char*          polling_code,
                                      struct ipcPipe       pipes) {
  agent_log(DEBUG, "Doing Mytoken Polling Code Lookup\n");

  char* data = generateMytokenPollingCodeLookupPostData(polling_code);
  if (data == NULL) {
    return oidc_errno;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* res =
      sendJSONPostWithoutBasicAuth(account_getMytokenEndpoint(account), data,
                                   account_getCertPath(account), NULL);
  secFree(data);
  if (res == NULL) {
    return oidc_errno;
  }

  parseTokenResponseCallbacks(0, res, account, &handleMytokenDeviceLookupError,
                              pipes, 0);
  secFree(res);
  return oidc_errno;
}
