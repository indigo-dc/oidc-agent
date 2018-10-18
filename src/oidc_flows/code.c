#include "code.h"

#include "../http/http.h"
#include "../httpserver/httpserver.h"
#include "../utils/portUtils.h"
#include "oidc.h"

#include <syslog.h>

oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing Authorization Code Flow\n");
  char* client_id     = account_getClientId(*account);
  char* client_secret = account_getClientSecret(*account);
  char* data          = generatePostData(
      "client_id", client_id, "client_secret", client_secret, "grant_type",
      "authorization_code", "code", code, "redirect_uri", used_redirect_uri,
      "response_type", "token");
  if (data == NULL) {
    secFree(client_id);
    secFree(client_secret);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(account_getTokenEndpoint(*account),
                                        data, account_getCertPath(*account),
                                        client_id, client_secret);
  secFree(data);
  secFree(client_id);
  secFree(client_secret);
  if (res == NULL) {
    return oidc_errno;
  }
  char* access_token = parseTokenResponse(res, account, 1, 1);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}

char* buildCodeFlowUri(struct oidc_account* account, char* state) {
  const char* auth_endpoint = account_getAuthorizationEndpoint(*account);
  list_t*     redirect_uris = account_getRedirectUris(*account);
  size_t      uri_count     = account_getRedirectUrisCount(*account);
  if (redirect_uris == NULL || uri_count <= 0) {
    oidc_errno = OIDC_ENOREURI;
    return NULL;
  }
  char* config = accountToJSONStringWithoutCredentials(*account);
  int   port =
      fireHttpServer(account_getRedirectUris(*account),
                     account_getRedirectUrisCount(*account), config, state);
  if (port <= 0) {
    secFree(config);
    return NULL;
  }
  secFree(config);
  char* redirect       = findRedirectUriByPort(*account, port);
  char* client_id      = account_getClientId(*account);
  char* uri_parameters = generatePostData(
      "response_type", "code", "client_id", client_id, "redirect_uri", redirect,
      "scope", account_getScope(*account), "access_type", "offline", "prompt",
      "consent", "state", state, NULL);
  secFree(redirect);
  secFree(client_id);
  char* uri = oidc_sprintf("%s?%s", auth_endpoint, uri_parameters);
  secFree(uri_parameters);
  return uri;
}
