#include "code.h"

#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc-agent/httpserver/startHttpserver.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/config/issuerConfig.h"
#include "utils/crypt/crypt.h"
#include "utils/listUtils.h"
#include "utils/oidc/oidcUtils.h"
#include "utils/portUtils.h"
#include "utils/string/stringUtils.h"
#include "utils/uriUtils.h"

oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri, char* code_verifier,
                          struct ipcPipe pipes) {
  agent_log(DEBUG, "Doing Authorization Code Flow\n");
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
  agent_log(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(account_getTokenEndpoint(account), data,
                                        account_getCertPathOrDefault(account),
                                        account_getClientId(account),
                                        account_getClientSecret(account));
  secFree(data);
  if (res == NULL) {
    return oidc_errno;
  }
  char* access_token =
      parseTokenResponse(TOKENPARSEMODE_RETURN_AT | TOKENPARSEMODE_SAVE_AT, res,
                         account, pipes, 0);
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
                       char** code_verifier_ptr, const unsigned char only_at) {
  const char* auth_endpoint = account_getAuthorizationEndpoint(account);
  list_t*     redirect_uris = account_getRedirectUris(account);
  size_t      uri_count     = account_getRedirectUrisCount(account);
  if (redirect_uris == NULL || uri_count <= 0) {
    oidc_errno = OIDC_ENOREURI;
    return NULL;
  }
  char* redirect =
      account_getNoScheme(account) ? NULL : findCustomSchemeUri(redirect_uris);
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
      if (strstarts(redirect, AGENT_CUSTOM_SCHEME) &&
          account_getNoScheme(account)) {
        char* tmp = list_at(redirect_uris, 1)->val;
        if (tmp != NULL) {
          redirect = tmp;
        }
      }
    }
    char* tmp = oidc_sprintf("%hhu:%s", strEnds(redirect, "/"), *state_ptr);
    secFree(*state_ptr);
    *state_ptr = tmp;
  }
  char*   scope = only_at ? removeScope(oidc_strcopy(account_getScope(account)),
                                        OIDC_SCOPE_OFFLINE_ACCESS)
                          : oidc_strcopy(account_getScope(account));
  list_t* postData = createList(
      LIST_CREATE_DONT_COPY_VALUES, OIDC_KEY_RESPONSETYPE,
      OIDC_RESPONSETYPE_CODE, OIDC_KEY_CLIENTID, account_getClientId(account),
      OIDC_KEY_REDIRECTURI, redirect, OIDC_KEY_SCOPE, scope, OIDC_KEY_PROMPT,
      OIDC_PROMPT_CONSENT, OIDC_KEY_STATE, *state_ptr, NULL);
  if (!only_at) {
    list_rpush(postData, list_node_new(GOOGLE_KEY_ACCESSTYPE));
    list_rpush(postData, list_node_new(GOOGLE_ACCESSTYPE_OFFLINE));
  }
  char* code_challenge_method = account_getCodeChallengeMethod(account);
  char* code_challenge =
      createCodeChallenge(*code_verifier_ptr, code_challenge_method);
  if (code_challenge) {
    list_rpush(postData, list_node_new(OIDC_KEY_CODECHALLENGE_METHOD));
    list_rpush(postData, list_node_new(code_challenge_method));
    list_rpush(postData, list_node_new(OIDC_KEY_CODECHALLENGE));
    list_rpush(postData, list_node_new(code_challenge));
  } else {
    secFree(*code_verifier_ptr);
    code_verifier_ptr = NULL;
  }
  char* aud_tmp = NULL;
  if (strValid(account_getAudience(account))) {
    if (getIssuerConfig(account_getIssuerUrl(account))->legacy_aud_mode) {
      list_rpush(postData, list_node_new(OIDC_KEY_AUDIENCE));
      list_rpush(postData, list_node_new(account_getAudience(account)));
    } else {
      aud_tmp = oidc_strcopy(account_getAudience(account));
      addAudienceRFC8707ToList(postData, aud_tmp);
    }
  }
  char* uri_parameters = generatePostDataFromList(postData);
  secFree(code_challenge);
  secFree(scope);
  secFree(aud_tmp);
  list_destroy(postData);
  char* uri = oidc_sprintf("%s?%s", auth_endpoint, uri_parameters);
  secFree(uri_parameters);
  return uri;
}
