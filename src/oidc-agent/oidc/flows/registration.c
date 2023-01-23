#include "registration.h"

#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "oidc-agent/http/http_ipc.h"
#include "utils/agentLogger.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/portUtils.h"
#include "utils/string/stringUtils.h"

char* generateRedirectUris() {
  char* redirect_uri0 = portToUri(HTTP_DEFAULT_PORT);
  char* redirect_uri1 = portToUri(getRandomPort());
  char* redirect_uri2 = portToUri(HTTP_FALLBACK_PORT);
  char* redirect_uri3 = oidc_strcat(AGENT_CUSTOM_SCHEME, "redirect");

  cJSON* json = generateJSONArray(redirect_uri0, redirect_uri1, redirect_uri2,
                                  redirect_uri3, NULL);
  secFree(redirect_uri0);
  secFree(redirect_uri1);
  secFree(redirect_uri2);
  secFree(redirect_uri3);
  char* uris = jsonToStringUnformatted(json);
  secFreeJson(json);
  return uris;
}

char* getRegistrationPostData(const struct oidc_account* account, list_t* flows,
                              const char* application_type) {
  char* client_name    = account_getClientName(account);
  char* response_types = getUsableResponseTypes(account, flows);
  char* grant_types    = getUsableGrantTypes(account, flows);
  char* redirect_uris_json =
      account_getRedirectUris(account)
          ? listToJSONArrayString(account_getRedirectUris(account))
          : generateRedirectUris();
  cJSON* json = generateJSONObject(
      OIDC_KEY_APPLICATIONTYPE, cJSON_String, application_type,
      OIDC_KEY_CLIENTNAME, cJSON_String, client_name, OIDC_KEY_RESPONSETYPES,
      cJSON_Array, response_types, OIDC_KEY_GRANTTYPES, cJSON_Array,
      grant_types, OIDC_KEY_SCOPE, cJSON_String, account_getScope(account),
      OIDC_KEY_REDIRECTURIS, cJSON_Array, redirect_uris_json, NULL);
  secFree(response_types);
  secFree(grant_types);
  secFree(redirect_uris_json);
  char* json_str = jsonToStringUnformatted(json);
  secFreeJson(json);
  return json_str;
}

char* _dynamicRegistration(struct oidc_account* account, list_t* flows,
                           const char* access_token,
                           const char* application_type) {
  agent_log(DEBUG, "Performing dynamic Registration flow");
  if (!strValid(account_getRegistrationEndpoint(account))) {
    oidc_errno = OIDC_ENOSUPREG;
    return NULL;
  }
  char* body = getRegistrationPostData(account, flows, application_type);
  if (body == NULL) {
    return NULL;
  }
  agent_log(DEBUG, "Data to send: %s", body);
  struct curl_slist* headers = NULL;
  if (strValid(access_token)) {
    char* auth_header =
        oidc_sprintf(HTTP_HEADER_AUTHORIZATION_BEARER_FMT, access_token);
    headers = curl_slist_append(headers, auth_header);
    secFree(auth_header);
  }
  char* res = sendJSONPostWithBasicAuth(
      account_getRegistrationEndpoint(account), body,
      account_getCertPathOrDefault(account), account_getClientId(account),
      account_getClientSecret(account), headers);
  secFree(body);
  if (res == NULL) {
    return NULL;
  }
  return res;
}

char* dynamicRegistration(struct oidc_account* account, list_t* flows,
                          const char* access_token) {
  char* res = _dynamicRegistration(account, flows, access_token,
                                   OIDC_APPLICATIONTYPES_WEB);
  if (res == NULL || !isJSONObject(res)) {
    return res;
  }
  char* error = getJSONValueFromString(res, OIDC_KEY_ERROR);
  char* error_description =
      getJSONValueFromString(res, OIDC_KEY_ERROR_DESCRIPTION);
  if (error == NULL) {
    return res;
  }
  if (strequal(error, "invalid redirect_uri") && error_description &&
      strequal(error_description,
               "Custom redirect_uri not allowed for web client")) {
    return _dynamicRegistration(account, flows, access_token,
                                OIDC_APPLICATIONTYPES_NATIVE);
  }
  return res;
}
