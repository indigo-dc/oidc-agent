#include "refresh.h"

#include <stddef.h>

#include "account/account.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/stringUtils.h"

char* generateRefreshPostData(const struct oidc_account* a, const char* scope,
                              const char* audience) {
  char* refresh_token = account_getRefreshToken(a);
  char* scope_tmp     = oidc_strcopy(
          strValid(scope) ? scope
                          : account_getScope(
                                a));  // if scopes are explicitly set use these, if
                                  // not we use the same as for the used refresh
                                  // token. Usually this parameter can be
                                  // omitted. For unity we have to include this.
  char* aud_tmp =
      oidc_strcopy(strValid(audience)
                       ? audience
                       : NULL);  // account_getAudience(a));  // if audience is
                                 // explicitly set use it, if not we use the
                                 // default audience for this account. This is
                                 // only needed if including audience changes
                                 // not only the audience of the new AT, but
                                 // also of the RT and therefore of future ATs.
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
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(p), data, account_getCertPath(p),
      account_getClientId(p), account_getClientSecret(p));
  secFree(data);
  if (NULL == res) {
    return NULL;
    ;
  }

  char* access_token = parseTokenResponse(
      return_mode |
          TOKENPARSEMODE_SAVE_AT_IF(!strValid(scope) && !strValid(audience)),
      res, p, pipes, 1);
  secFree(res);
  return access_token;
}
