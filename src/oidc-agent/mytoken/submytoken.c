#include "submytoken.h"

#include <string.h>

#include "account/account.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/mytoken_values.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/oidcd/oidcd_handler.h"
#include "profile.h"
#include "utils/agentLogger.h"
#include "utils/crypt/crypt.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"
#include "utils/parseJson.h"
#include "utils/string/stringUtils.h"

char* get_submytoken(struct ipcPipe pipes, struct oidc_account* account,
                     const char* profile, const char* application_hint) {
  agent_log(DEBUG, "Obtaining sub-mytoken");
  const char* mytoken_endpoint = account_getMytokenEndpoint(account);
  if (!strValid(mytoken_endpoint)) {
    oidc_errno = OIDC_ENOMYTOKEN;
    return NULL;
  }
  if (profile == NULL) {
    profile = "web-default";
  }
  cJSON* json = parseUsedMytokenProfile(profile);
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
  char* consent_endpoint = oidc_pathcat(account_getMytokenUrl(account), "c");
  char* consent          = sendJSONPostWithoutBasicAuth(
      consent_endpoint, data, account_getCertPath(account), NULL);
  secFree(consent_endpoint);
  secFree(data);
  if (consent == NULL) {
    return NULL;
  }
  if (isJSONObject(consent)) {
    char* error = parseForError(consent);
    if (error) {
      oidc_setInternalError(error);
      secFree(error);
      return NULL;
    }
  }
  char* base64      = toBase64(consent, strlen(consent));
  char* prompt_data = oidc_sprintf("%lu:%s", strlen(consent), base64);
  secFree(consent);
  secFree(base64);
  char* updated_data = _oidcd_getMytokenConfirmation(pipes, prompt_data);
  secFree(prompt_data);

  json = cJSON_Parse(updated_data);
  if (json == NULL) {
    return NULL;
  }
  setJSONValue(json, "oidc_issuer", account_getIssuerUrl(account));
  setJSONValue(json, OIDC_KEY_GRANTTYPE, MYTOKEN_GRANTTYPE_MYTOKEN);
  setJSONValue(json, "mytoken", account_getRefreshToken(account));
  updated_data = jsonToStringUnformatted(json);
  secFree(json);

  agent_log(DEBUG, "Sending updated data: %s", updated_data);
  char* res = sendJSONPostWithoutBasicAuth(mytoken_endpoint, updated_data,
                                           account_getCertPath(account), NULL);
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
  setJSONValue(m, IPC_KEY_MYTOKEN_OIDC_ISS, account_getIssuerUrl(account));
  setJSONValue(m, IPC_KEY_MYTOKEN_MY_ISS, account_getMytokenUrl(account));
  char* r = jsonToStringUnformatted(m);
  secFreeJson(m);
  return r;
}
