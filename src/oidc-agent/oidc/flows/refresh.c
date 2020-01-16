#include "refresh.h"

#include "account/account.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/stringUtils.h"

#include <stddef.h>

char* generateRefreshPostData(const struct oidc_account* a, const char* scope,
                              const char* audience) {
  char* refresh_token = account_getRefreshToken(a);
  char* scope_tmp     = oidc_strcopy(
      strValid(scope) ? scope
                      : account_getScope(
                            a));  // if scopes are explicilty set use these, if
                                  // not we use the same as for the used refresh
                                  // token. Usually this parameter can be
                                  // omitted. For unity we have to include this.
  list_t* postDataList = list_new();
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTID));
  // list_rpush(postDataList, list_node_new(account_getClientId(a)));
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTSECRET));
  // list_rpush(postDataList, list_node_new(account_getClientSecret(a)));
  list_rpush(postDataList, list_node_new(OIDC_KEY_GRANTTYPE));
  list_rpush(postDataList, list_node_new(OIDC_GRANTTYPE_REFRESH));
  list_rpush(postDataList, list_node_new(OIDC_KEY_REFRESHTOKEN));
  list_rpush(postDataList, list_node_new(refresh_token));
  if (strValid(scope_tmp)) {
    list_rpush(postDataList, list_node_new(OIDC_KEY_SCOPE));
    list_rpush(postDataList, list_node_new(scope_tmp));
  }
  char* aud_tmp = oidc_strcopy(audience);
  if (strValid(aud_tmp)) {
    list_rpush(postDataList, list_node_new(OIDC_KEY_AUDIENCE));
    list_rpush(postDataList, list_node_new(aud_tmp));
  }
  char* str = generatePostDataFromList(postDataList);
  list_destroy(postDataList);
  secFree(aud_tmp);
  secFree(scope_tmp);
  return str;
}

/** @fn oidc_error_t refreshFlow(struct oidc_account* p)
 * @brief issues an access token via refresh flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
char* refreshFlow(struct oidc_account* p, const char* scope,
                  const char* audience, struct ipcPipe pipes) {
  agent_log(DEBUG, "Doing RefreshFlow\n");
  char* data = generateRefreshPostData(p, scope, audience);
  if (data == NULL) {
    return NULL;
    ;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(p), data, account_getCertPath(p),
      account_getClientId(p), account_getClientSecret(p));
  secFree(data);
  if (NULL == res) {
    return NULL;
    ;
  }

  char* access_token = parseTokenResponse(res, p, !strValid(scope), pipes);
  secFree(res);
  return access_token;
}
