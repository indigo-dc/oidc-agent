#include "registration.h"

#include "account/account.h"
#include "account/issuer_helper.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/httpserver/startHttpserver.h"
#include "utils/json.h"
#include "utils/portUtils.h"
#include "utils/stringUtils.h"

#include <syslog.h>

char* generateRedirectUris() {
  char*  redirect_uri0 = portToUri(HTTP_DEFAULT_PORT);
  char*  redirect_uri1 = portToUri(getRandomPort());
  char*  redirect_uri2 = portToUri(HTTP_FALLBACK_PORT);
  cJSON* json =
      generateJSONArray(redirect_uri0, redirect_uri1, redirect_uri2, NULL);
  secFree(redirect_uri0);
  secFree(redirect_uri1);
  secFree(redirect_uri2);
  char* uris = jsonToString(json);
  secFreeJson(json);
  return uris;
}

char* getRegistrationPostData(struct oidc_account account, list_t* flows) {
  char* client_name    = account_getClientName(account);
  char* response_types = getUsableResponseTypes(account, flows);
  char* grant_types =
      getUsableGrantTypes(account_getGrantTypesSupported(account), flows);
  char*  redirect_uris_json = generateRedirectUris();
  cJSON* json               = generateJSONObject(
      "application_type", cJSON_String, "web", "client_name", cJSON_String,
      client_name, "response_types", cJSON_Array, response_types, "grant_types",
      cJSON_Array, grant_types, "scope", cJSON_String,
      account_getScope(account), "redirect_uris", cJSON_Array,
      redirect_uris_json, NULL);
  secFree(response_types);
  secFree(grant_types);
  secFree(redirect_uris_json);
  char* json_str = jsonToString(json);
  secFreeJson(json);
  return json_str;
}

char* dynamicRegistration(struct oidc_account* account, list_t* flows,
                          const char* access_token) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Performing dynamic Registration flow");
  if (!strValid(account_getRegistrationEndpoint(*account))) {
    oidc_seterror(
        "Dynamic registration is not supported by this issuer. Please register "
        "a client manually and then run oidc-gen with the -m flag.");
    oidc_errno = OIDC_EERROR;
    return NULL;
  }
  char* body = getRegistrationPostData(*account, flows);
  if (body == NULL) {
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", body);
  struct curl_slist* headers =
      curl_slist_append(NULL, "Content-Type: application/json");
  if (strValid(access_token)) {
    char* auth_header = oidc_sprintf("Authorization: Bearer %s", access_token);
    headers           = curl_slist_append(headers, auth_header);
    secFree(auth_header);
  }
  char* res =
      httpsPOST(account_getRegistrationEndpoint(*account), body, headers,
                account_getCertPath(*account), account_getClientId(*account),
                account_getClientSecret(*account));
  curl_slist_free_all(headers);
  secFree(body);
  if (res == NULL) {
    return NULL;
  }
  return res;
}
