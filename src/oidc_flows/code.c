#include "code.h"

#include "../http/http.h"
#include "../httpserver/httpserver.h"
#include "../utils/portUtils.h"
#include "oidc.h"

#include <syslog.h>

oidc_error_t codeExchange(struct oidc_account* account, const char* code,
                          const char* used_redirect_uri) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Doing Authorization Code Flow\n");
  char* data = generatePostData(
      "client_id", account_getClientId(*account), "client_secret",
      account_getClientSecret(*account), "grant_type", "authorization_code",
      "code", code, "redirect_uri", used_redirect_uri, "response_type",
      "token");
  if (data == NULL) {
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(*account), data, account_getCertPath(*account),
      account_getClientId(*account), account_getClientSecret(*account));
  clearFreeString(data);
  if (res == NULL) {
    return oidc_errno;
  }
  char* access_token = parseTokenResponse(res, account, 1, 1);
  clearFreeString(res);
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
  size_t         i = 0;
  unsigned short ports[account_getRedirectUrisCount(*account)];
  for (i = 0; i < sizeof(ports) / sizeof(*ports); i++) {
    ports[i] = getPortFromUri(list_at(redirect_uris, i)->val);
  }
  char* config = accountToJSONWithoutCredentials(*account);
  int   port =
      fireHttpServer(ports, sizeof(ports) / sizeof(*ports), config, state);
  if (port <= 0) {
    clearFreeString(config);
    return NULL;
  }
  clearFreeString(config);
  char* redirect       = portToUri(port);
  char* uri_parameters = generatePostData(
      "response_type", "code", "client_id", account_getClientId(*account),
      "redirect_uri", redirect, "scope", account_getScope(*account),
      "access_type", "offline", "prompt", "consent", "state", state, NULL);
  clearFreeString(redirect);
  char* uri = oidc_sprintf("%s?%s", auth_endpoint, uri_parameters);
  clearFreeString(uri_parameters);
  return uri;
}
