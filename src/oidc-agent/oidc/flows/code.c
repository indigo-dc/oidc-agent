#include "code.h"

#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/httpserver/startHttpserver.h"
#include "oidc.h"
#include "utils/crypt/crypt.h"
#include "utils/listUtils.h"
#include "utils/portUtils.h"
#include "utils/uriUtils.h"

#include <syslog.h>

oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri, char* code_verifier,
                          struct ipcPipe pipes) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing Authorization Code Flow\n");
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
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
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
  if (redirect == NULL) {
    int port = fireHttpServer(account_getRedirectUris(account),
                              account_getRedirectUrisCount(account), state_ptr);
    if (port <= 0) {
      return NULL;
    }
    redirect = findRedirectUriByPort(account, port);
  } else {  // custome scheme url
    char* tmp = oidc_sprintf("%hhu:%s", strEnds(redirect, "/"), *state_ptr);
    secFree(*state_ptr);
    *state_ptr = tmp;
  }
  list_t* postData = createList(
      LIST_CREATE_DONT_COPY_VALUES, OIDC_KEY_RESPONSETYPE,
      OIDC_RESPONSETYPE_CODE, OIDC_KEY_CLIENTID, account_getClientId(account),
      OIDC_KEY_REDIRECTURI, redirect, OIDC_KEY_SCOPE, account_getScope(account),
      GOOGLE_KEY_ACCESSTYPE, GOOGLE_ACCESSTYPE_OFFLINE, OIDC_KEY_PROMPT,
      OIDC_PROMPT_CONSENT, OIDC_KEY_STATE, *state_ptr, NULL);
  char* code_challenge_method = account_getCodeChallengeMethod(account);
  char* code_challenge =
      createCodeChallenge(code_verifier, code_challenge_method);
  if (code_challenge) {
    list_rpush(postData, list_node_new(OIDC_KEY_CODECHALLENGE_METHOD));
    list_rpush(postData, list_node_new(code_challenge_method));
    list_rpush(postData, list_node_new(OIDC_KEY_CODECHALLENGE));
    list_rpush(postData, list_node_new(code_challenge));
  }
  char* uri_parameters = generatePostDataFromList(postData);
  secFree(code_challenge);
  list_destroy(postData);
  char* uri = oidc_sprintf("%s?%s", auth_endpoint, uri_parameters);
  secFree(uri_parameters);
  return uri;
}
