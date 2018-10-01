#include "registration.h"

#include "../json.h"
#include "../account.h"
#include "../http/http.h"
#include "../httpserver/httpserver.h"
#include "../issuer_helper.h"
#include "../utils/stringUtils.h"
#include "../utils/portUtils.h"

#include <syslog.h>

char* generateRedirectUris() {
  char* redirect_uri0 = portToUri(HTTP_DEFAULT_PORT);
  char* redirect_uri1 = portToUri(getRandomPort());
  char* redirect_uri2 = portToUri(HTTP_FALLBACK_PORT);
  char* uris = generateJSONArray(redirect_uri0, redirect_uri1, redirect_uri2, NULL);
  clearFreeString(redirect_uri0);
  clearFreeString(redirect_uri1);
  clearFreeString(redirect_uri2);
  return uris;
}

char* getRegistrationPostData(struct oidc_account account, int usePasswordGrantType) {
  char* client_name = oidc_sprintf("oidc-agent:%s", account_getName(account));
  if(client_name == NULL) {
    return NULL;
  }
  char* response_types = getUsableResponseTypes(account, usePasswordGrantType);
  char* grant_types = getUsableGrantTypes(account_getGrantTypesSupported(account), usePasswordGrantType);
  char* redirect_uris_json = generateRedirectUris();
  char* json = generateJSONObject(
      "application_type", "web", 1,
      "client_name", client_name, 1,
      "response_types", response_types, 0,
      "grant_types", grant_types, 0,
      "scope", account_getScope(account), 1,
      "redirect_uris", redirect_uris_json, 0,
      NULL
      );
  clearFreeString(client_name);
  clearFreeString(response_types);
  clearFreeString(grant_types);
  clearFreeString(redirect_uris_json);
  return json;
}

char* dynamicRegistration(struct oidc_account* account, int usePasswordGrantType, const char* access_token) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Performing dynamic Registration flow");
  if(!strValid(account_getRegistrationEndpoint(*account))) {
    oidc_seterror("Dynamic registration is not supported by this issuer. Please register a client manually and then run oidc-gen with the -m flag.");
    oidc_errno = OIDC_EERROR;
    return NULL;
  }
  char* body = getRegistrationPostData(*account, usePasswordGrantType);
  struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
  if(strValid(access_token)) {
    char* auth_header = oidc_sprintf("Authorization: Bearer %s", access_token);
    headers = curl_slist_append(headers, auth_header);
    clearFreeString(auth_header);
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Data to send: %s", body);
  char* res = httpsPOST(account_getRegistrationEndpoint(*account), body, headers, account_getCertPath(*account), account_getClientId(*account), account_getClientSecret(*account));
  curl_slist_free_all(headers);
  clearFreeString(body);
  if(res==NULL) {
    return NULL;
  }
  return res;
}

