#include "password.h"

#include <stddef.h>

#include "account/account.h"
#include "defines/oidc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "oidc.h"
#include "utils/agentLogger.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

char* generatePasswordPostData(const struct oidc_account* a,
                               const char*                scope) {
  list_t* postDataList = list_new();
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTID));
  // list_rpush(postDataList, list_node_new(account_getClientId(a)));
  // list_rpush(postDataList, list_node_new(OIDC_KEY_CLIENTSECRET));
  // list_rpush(postDataList, list_node_new(account_getClientSecret(a)));
  list_rpush(postDataList, list_node_new(OIDC_KEY_GRANTTYPE));
  list_rpush(postDataList, list_node_new(OIDC_GRANTTYPE_PASSWORD));
  list_rpush(postDataList, list_node_new(OIDC_KEY_USERNAME));
  list_rpush(postDataList, list_node_new(account_getUsername(a)));
  list_rpush(postDataList, list_node_new(OIDC_KEY_PASSWORD));
  list_rpush(postDataList, list_node_new(account_getPassword(a)));
  if (scope || strValid(account_getScope(a))) {
    list_rpush(postDataList, list_node_new(OIDC_KEY_SCOPE));
    list_rpush(postDataList,
               list_node_new((char*)scope ?: account_getScope(a)));
  }
  if (strValid(account_getAudience(a))) {
    list_rpush(postDataList, list_node_new(OIDC_KEY_AUDIENCE));
    list_rpush(postDataList, list_node_new(account_getAudience(a)));
  }
  char* str = generatePostDataFromList(postDataList);
  list_destroy(postDataList);
  return str;
}

/** @fn oidc_error_t passwordFlow(struct oidc_account* p)
 * @brief issues an access token using the password flow
 * @param p a pointer to the account for whom an access token should be issued
 * @return 0 on success; 1 otherwise
 */
oidc_error_t passwordFlow(struct oidc_account* p, struct ipcPipe pipes,
                          const char* scope) {
  agent_log(DEBUG, "Doing PasswordFlow\n");
  char* data = generatePasswordPostData(p, scope);
  if (data == NULL) {
    return oidc_errno;
    ;
  }
  agent_log(DEBUG, "Data to send: %s", data);
  char* res = sendPostDataWithBasicAuth(
      account_getTokenEndpoint(p), data, account_getCertPathOrDefault(p),
      account_getClientId(p), account_getClientSecret(p));
  secFree(data);
  if (NULL == res) {
    return oidc_errno;
  }

  char* access_token = parseTokenResponse(
      TOKENPARSEMODE_RETURN_AT | TOKENPARSEMODE_SAVE_AT, res, p, pipes, 0);
  secFree(res);
  return access_token == NULL ? oidc_errno : OIDC_SUCCESS;
}
