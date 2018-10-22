#include "code.h"

#include "../http/http_ipc.h"
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
  secFree(data);
  if (res == NULL) {
    return oidc_errno;
  }
  char* access_token = parseTokenResponse(res, account, 1, 1);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}

char* buildCodeFlowUri(const struct oidc_account* account, char* state) {
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
  char* uri_parameters = generatePostData(
      "response_type", "code", "client_id", account_getClientId(*account),
      "redirect_uri", redirect, "scope", account_getScope(*account),
      "access_type", "offline", "prompt", "consent", "state", state, NULL);
  char* uri = oidc_sprintf("%s?%s", auth_endpoint, uri_parameters);
  secFree(uri_parameters);
  return uri;
}
