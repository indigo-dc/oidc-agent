#include "code.h"

#include "account/issuer_helper.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/httpserver/startHttpserver.h"
#include "oidc-agent/oidcd/jose/joseUtils.h"
#include "oidc.h"
#include "utils/crypt/crypt.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/portUtils.h"
#include "utils/uriUtils.h"

oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri, char* code_verifier,
                          struct ipcPipe pipes) {
  logger(DEBUG, "Doing Authorization Code Flow\n");
  list_t* postData =
      createList(LIST_CREATE_DONT_COPY_VALUES,
                 // OIDC_KEY_CLIENTID, account_getClientId(account),
                 // OIDC_KEY_CLIENTSECRET, account_getClientSecret(account),
                 OIDC_KEY_GRANTTYPE, OIDC_GRANTTYPE_AUTHCODE, OIDC_KEY_CODE,
                 code, OIDC_KEY_REDIRECTURI, used_redirect_uri,
                 OIDC_KEY_RESPONSETYPE, OIDC_RESPONSETYPE_TOKEN, NULL);
  if (code_verifier) {
    list_rpush(postData, list_node_new(OIDC_KEY_CODEVERIFIER));
    list_rpush(postData, list_node_new(code_verifier));
  }
  char* data = generatePostDataFromList(postData);
  list_destroy(postData);
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
  char* access_token = parseTokenResponse(res, account, 1, pipes);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}

char* createCodeChallenge(const char* code_verifier,
                          const char* code_challenge_method) {
  char* code_challenge = NULL;
  if (strValid(code_challenge_method)) {
    if (strequal(code_challenge_method, CODE_CHALLENGE_METHOD_PLAIN)) {
      code_challenge = oidc_strcopy(code_verifier);
    } else if (strequal(code_challenge_method, CODE_CHALLENGE_METHOD_S256)) {
      code_challenge = s256(code_verifier);
    }
  }
  return code_challenge;
}

char* buildCodeFlowUri(const struct oidc_account* account, char** state_ptr,
                       const char* code_verifier) {
  const char* auth_endpoint = account_getAuthorizationEndpoint(account);
  list_t*     redirect_uris = account_getRedirectUris(account);
  size_t      uri_count     = account_getRedirectUrisCount(account);
  if (redirect_uris == NULL || uri_count <= 0) {
    oidc_errno = OIDC_ENOREURI;
    return NULL;
  }
  char* redirect = findCustomSchemeUri(redirect_uris);
  if (!account_getNoWebServer(account)) {
    int port = fireHttpServer(account_getRedirectUris(account),
                              account_getRedirectUrisCount(account), state_ptr);
    if (port <= 0) {
      if (redirect == NULL) {
        return NULL;
      }
    }
    redirect = findRedirectUriByPort(account, port);
  } else {                   // no web server
    if (redirect == NULL) {  // no custom scheme uri found
      redirect = list_at(redirect_uris, 0)->val;
    }
    char* tmp = oidc_sprintf("%hhu:%s", strEnds(redirect, "/"), *state_ptr);
    secFree(*state_ptr);
    *state_ptr = tmp;
  }
  list_t* postData = createList(LIST_CREATE_DONT_COPY_VALUES,
                                OIDC_KEY_RESPONSETYPE, OIDC_RESPONSETYPE_CODE,
                                OIDC_KEY_CLIENTID, account_getClientId(account),
                                OIDC_KEY_SCOPE, account_getScope(account),
                                OIDC_KEY_REDIRECTURI, redirect, NULL);
  if (compIssuerUrls(GOOGLE_ISSUER_URL, account_getIssuerUrl(account))) {
    list_rpush(postData, list_node_new(GOOGLE_KEY_ACCESSTYPE));
    list_rpush(postData, list_node_new(GOOGLE_ACCESSTYPE_OFFLINE));
  }
  char* code_challenge_method = account_getCodeChallengeMethod(account);
  char* code_challenge =
      createCodeChallenge(code_verifier, code_challenge_method);
  if (code_challenge) {
    list_rpush(postData, list_node_new(OIDC_KEY_CODECHALLENGE_METHOD));
    list_rpush(postData, list_node_new(code_challenge_method));
    list_rpush(postData, list_node_new(OIDC_KEY_CODECHALLENGE));
    list_rpush(postData, list_node_new(code_challenge));
  }
  if (!account_getJoseIsEnabled(account)) {
    list_rpush(postData, list_node_new(OIDC_KEY_PROMPT));
    list_rpush(postData, list_node_new(OIDC_PROMPT_CONSENT));
    list_rpush(postData, list_node_new(OIDC_KEY_STATE));
    list_rpush(postData, list_node_new(*state_ptr));
  }
  char* uri_parameters = generatePostDataFromList(postData);
  secFree(code_challenge);
  list_destroy(postData);
  if (account_getJoseIsEnabled(account)) {
    cJSON* json = postFormDataToJSONObject(uri_parameters);
    if (json == NULL) {
      secFree(uri_parameters);
      return NULL;
    }
    jsonAddStringValue(json, OIDC_KEY_PROMPT, OIDC_PROMPT_CONSENT);
    jsonAddStringValue(json, OIDC_KEY_STATE, *state_ptr);
    jsonAddStringValue(json, JWT_KEY_ISSUER, account_getClientId(account));
    jsonAddStringValue(json, JWT_KEY_AUDIENCE, account_getIssuerUrl(account));
    char* json_str = jsonToStringUnformatted(json);
    secFreeJson(json);
    char* request = jose_sign(json_str, account);
    secFree(json_str);
    if (request == NULL) {
      secFree(uri_parameters);
      return NULL;
    }
    char* uri_parameters_new =
        oidc_sprintf("%s&request=%s", uri_parameters, request);
    secFree(request);
    secFree(uri_parameters);
    uri_parameters = uri_parameters_new;
  }
  char* uri = oidc_sprintf("%s?%s", auth_endpoint, uri_parameters);
  secFree(uri_parameters);
  return uri;
}
