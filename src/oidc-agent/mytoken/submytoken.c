#include "submytoken.h"

#include "account/account.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/mytoken_values.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

char* get_submytoken(struct oidc_account* account, const char* profile,
                     const char* application_hint) {
  agent_log(DEBUG, "Obtaining sub-mytoken");
  const char* mytoken_endpoint = account_getMytokenEndpoint(account);
  if (!strValid(mytoken_endpoint)) {
    oidc_errno = OIDC_ENOMYTOKEN;
    return NULL;
  }
  cJSON* json = profile ? cJSON_Parse(profile) : cJSON_CreateObject();
  if (json == NULL) {
    return NULL;
  }
  setJSONValue(json, "oidc_issuer", account_getIssuerUrl(account));
  setJSONValue(json, OIDC_KEY_GRANTTYPE, MYTOKEN_GRANTTYPE_MYTOKEN);
  setJSONValue(json, "mytoken", account_getRefreshToken(account));
  char* name = application_hint
                   ? oidc_sprintf("mytoken requested by %s", application_hint)
                   : NULL;
  setJSONValueIfNotSet(json, "name", name);
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
  cJSON* m = cJSON_Parse(res);
  secFree(res);
  cJSON_DeleteItemFromObjectCaseSensitive(m, "updated_token");
  cJSON* expires_in = cJSON_DetachItemFromObject(m, OIDC_KEY_EXPIRESIN);
  if (expires_in) {
    time_t expires_at = time(NULL) + (time_t)cJSON_GetNumberValue(expires_in);
    secFreeJson(expires_in);
    cJSON_AddNumberToObject(m, AGENT_KEY_EXPIRESAT, (double)expires_at);
  }
  setJSONValue(m, IPC_KEY_STATUS, STATUS_SUCCESS);
  char* r = jsonToStringUnformatted(m);
  secFreeJson(m);
  return r;
}
