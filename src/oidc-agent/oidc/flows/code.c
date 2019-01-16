#include "code.h"

#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/httpserver/startHttpserver.h"
#include "oidc-agent/oidc/values.h"
#include "oidc.h"
#include "utils/crypt/crypt.h"
#include "utils/listUtils.h"
#include "utils/portUtils.h"

#include <syslog.h>

oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri, char* code_verifier) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing Authorization Code Flow\n");
  list_t* postData =
      createList(0, "client_id", account_getClientId(account), "client_secret",
                 account_getClientSecret(account), "grant_type",
                 "authorization_code", "code", code, "redirect_uri",
                 used_redirect_uri, "response_type", "token", NULL);
  if (code_verifier) {
    list_rpush(postData, list_node_new("code_verifier"));
    list_rpush(postData, list_node_new(code_verifier));
  }
  char* data = generatePostDataFromList(postData);
  list_destroy(postData);
  if (data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(account), data, account_getCertPath(account),
      account_getClientId(account), account_getClientSecret(account));
  secFree(data);
  if (res == NULL) {
    return oidc_errno;
  }
  char* access_token = parseTokenResponse(res, account, 1, 1);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}

char* buildCodeFlowUri(const struct oidc_account* account, const char* state,
                       const char* code_verifier) {
  const char* auth_endpoint = account_getAuthorizationEndpoint(account);
  list_t*     redirect_uris = account_getRedirectUris(account);
  size_t      uri_count     = account_getRedirectUrisCount(account);
  if (redirect_uris == NULL || uri_count <= 0) {
    oidc_errno = OIDC_ENOREURI;
    return NULL;
  }
  char* config = accountToJSONStringWithoutCredentials(account);
  int   port   = fireHttpServer(account_getRedirectUris(account),
                            account_getRedirectUrisCount(account), config,
                            state, code_verifier);
  if (port <= 0) {
    secFree(config);
    return NULL;
  }
  secFree(config);
  char*   redirect = findRedirectUriByPort(account, port);
  list_t* postData = createList(
      0, "response_type", "code", "client_id", account_getClientId(account),
      "redirect_uri", redirect, "scope", account_getScope(account),
      "access_type", "offline", "prompt", "consent", "state", state, NULL);
  char* code_challenge        = NULL;
  char* code_challenge_method = account_getCodeChallengeMethod(account);
  if (strValid(code_challenge_method)) {
    list_rpush(postData, list_node_new("code_challenge_method"));
    list_rpush(postData, list_node_new(code_challenge_method));
    if (strequal(code_challenge_method, CODE_CHALLENGE_METHOD_PLAIN)) {
      code_challenge = oidc_strcopy(code_verifier);
    } else if (strequal(code_challenge_method, CODE_CHALLENGE_METHOD_S256)) {
      code_challenge = s256(code_verifier);
    }
    list_rpush(postData, list_node_new("code_challenge"));
    list_rpush(postData, list_node_new(code_challenge));
  }
  char* uri_parameters = generatePostDataFromList(postData);
  secFree(code_challenge);
  list_destroy(postData);
  char* uri = oidc_sprintf("%s?%s", auth_endpoint, uri_parameters);
  secFree(uri_parameters);
  return uri;
}
