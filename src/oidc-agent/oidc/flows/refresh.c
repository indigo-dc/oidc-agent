#include "refresh.h"

#include <stddef.h>

#include "account/account.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/config/issuerConfig.h"
#include "utils/string/stringUtils.h"

char* generateRefreshPostData(const struct oidc_account* a, const char* scope,
                              const char* audience) {
  char* refresh_token = account_getRefreshToken(a);
  if (NULL == scope) {
    scope = strValid(account_getRefreshScope(a)) ? account_getRefreshScope(a)
                                                 : account_getAuthScope(a);
    // if scopes are explicitly set use these, if not we use the same as for the
    // used refresh token. Usually this parameter can be omitted. For unity we
    // have to include this.
  }
  list_t* postDataList = list_new();
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTID));
  // list_rpush(postDataList, list_node_new(account_getClientId(a)));
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTSECRET));
  // list_rpush(postDataList, list_node_new(account_getClientSecret(a)));
  list_rpush(postDataList, list_node_new(OIDC_KEY_GRANTTYPE));
  list_rpush(postDataList, list_node_new(OIDC_GRANTTYPE_REFRESH));
  list_rpush(postDataList, list_node_new(OIDC_KEY_REFRESHTOKEN));
  list_rpush(postDataList, list_node_new(refresh_token));
  if (account_getMytokenUrl(a) == NULL) {
    if (!strValid(account_getClientSecret(a)) ||
        account_getUsesPubClient(
            a)) {  // In case of public client add client id to request
      list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTID));
      list_rpush(postDataList, list_node_new(account_getClientId(a)));
      if (strValid(account_getClientSecret(a))) {
        list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTSECRET));
        list_rpush(postDataList, list_node_new(account_getClientSecret(a)));
      }
    }
  }

  if (strValid(scope)) {
    list_rpush(postDataList, list_node_new(OIDC_KEY_SCOPE));
    list_rpush(postDataList, list_node_new((void*)scope));
  }
  char* aud_tmp = NULL;
  if (strValid(audience)) {
    aud_tmp                          = oidc_strcopy(audience);
    const struct issuerConfig* iss_c = getIssuerConfig(account_getIssuerUrl(a));
    if (iss_c && iss_c->legacy_aud_mode) {
      list_rpush(postDataList, list_node_new(OIDC_KEY_AUDIENCE));
      list_rpush(postDataList, list_node_new(aud_tmp));
    } else {
      addAudienceRFC8707ToList(postDataList, aud_tmp);
    }
  }
  char* str = generatePostDataFromList(postDataList);
  list_destroy(postDataList);
  secFree(aud_tmp);
  return str;
}

char* refreshFlow(unsigned char return_mode, struct oidc_account* p,
                  const char* scope, const char* audience,
                  struct ipcPipe pipes) {
  agent_log(DEBUG, "Doing RefreshFlow\n");
  char* data = generateRefreshPostData(p, scope, audience);
  if (data == NULL) {
    return NULL;
    ;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* cert_path = account_getCertPathOrDefault(p);
  char* res       = sendPostDataWithBasicAuth(account_getTokenEndpoint(p), data,
                                              cert_path, account_getClientId(p),
                                              account_getClientSecret(p));
  secFree(cert_path);
  secFree(data);
  if (NULL == res) {
    return NULL;
  }

  char* access_token = parseTokenResponse(
      return_mode |
          TOKENPARSEMODE_SAVE_AT_IF(!strValid(scope) && !strValid(audience)),
      res, p, pipes, 1);
  secFree(res);
  return access_token;
}
