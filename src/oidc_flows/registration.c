#include "registration.h"

#include "../account.h"
#include "../http/http.h"
#include "../httpserver/httpserver.h"
#include "../issuer_helper.h"
#include "../json.h"
#include "../utils/portUtils.h"
#include "../utils/stringUtils.h"

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

char* getRegistrationPostData(struct oidc_account account,
                              int                 usePasswordGrantType) {
  char* client_name    = account_getClientName(account);
  char* response_types = getUsableResponseTypes(account, usePasswordGrantType);
  char* grant_types    = getUsableGrantTypes(
      account_getGrantTypesSupported(account), usePasswordGrantType);
  char*  redirect_uris_json = generateRedirectUris();
  cJSON* json               = generateJSONObject(
      "application_type", "web", cJSON_String, "client_name", client_name,
      cJSON_String, "response_types", response_types, cJSON_Array,
      "grant_types", grant_types, cJSON_Array, "scope",
      account_getScope(account), cJSON_String, "redirect_uris",
      redirect_uris_json, cJSON_Array, NULL);
  secFree(response_types);
  secFree(grant_types);
  secFree(redirect_uris_json);
  char* json_str = jsonToString(json);
  secFreeJson(json);
  return json_str;
}

char* dynamicRegistration(struct oidc_account* account,
                          int usePasswordGrantType, const char* access_token) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Performing dynamic Registration flow");
  if (!strValid(account_getRegistrationEndpoint(*account))) {
    oidc_seterror(
        "Dynamic registration is not supported by this issuer. Please register "
        "a client manually and then run oidc-gen with the -m flag.");
    oidc_errno = OIDC_EERROR;
    return NULL;
  }
  char* body = getRegistrationPostData(*account, usePasswordGrantType);
  struct curl_slist* headers =
      curl_slist_append(NULL, "Content-Type: application/json");
  if (strValid(access_token)) {
    char* auth_header = oidc_sprintf("Authorization: Bearer %s", access_token);
    headers           = curl_slist_append(headers, auth_header);
    secFree(auth_header);
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", body);
  char* client_id     = account_getClientId(*account);
  char* client_secret = account_getClientSecret(*account);
  char* res =
      httpsPOST(account_getRegistrationEndpoint(*account), body, headers,
                account_getCertPath(*account), client_id, client_secret);
  secFree(client_id);
  secFree(client_secret);
  curl_slist_free_all(headers);
  secFree(body);
  if (res == NULL) {
    return NULL;
  }
  return res;
}
