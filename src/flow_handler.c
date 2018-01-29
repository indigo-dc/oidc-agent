#define _GNU_SOURCE

#include "flow_handler.h"
#include "oidc.h"
#include "ipc_values.h"

#include "../lib/list/src/list.h"

#include <string.h>
#include <syslog.h>

char* getAccessTokenUsingRefreshFlow(struct oidc_account* account, time_t min_valid_period, const char* scope) {
  if(scope==NULL && min_valid_period!=FORCE_NEW_TOKEN && isValid(account_getAccessToken(*account)) && tokenIsValidForSeconds(*account, min_valid_period)) {
    return account_getAccessToken(*account);
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "No acces token found that is valid long enough");
  return tryRefreshFlow(account, scope);
}

oidc_error_t getAccessTokenUsingPasswordFlow(struct oidc_account* account) {
  if(isValid(account_getAccessToken(*account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno = tryPasswordFlow(account);
  return oidc_errno;
}

oidc_error_t getAccessTokenUsingAuthCodeFlow(struct oidc_account* account, const char* code, const char* used_redirect_uri) {
  if(isValid(account_getAccessToken(*account))) {
    return OIDC_SUCCESS;
  }
  oidc_errno = codeExchange(account, code, used_redirect_uri);
  return oidc_errno;
}

oidc_error_t getAccessTokenUsingDeviceFlow(struct oidc_account* account) {
  if(isValid(account_getAccessToken(*account))) {
    return OIDC_SUCCESS;
  }
  // oidc_errno = tryDeviceFlow(account);
  oidc_errno = OIDC_NOTIMPL;
  return oidc_errno;
}

struct flow_order {
  unsigned char refresh;
  unsigned char password;
  unsigned char code;
  unsigned char device;
};

list_t* parseFlow(const char* flow) {
  list_t* flows = list_new();
  flows->match = (int(*) (void*, void*)) &strequal;
  if(flow==NULL) { //Using defualt order
    list_rpush(flows, list_node_new(FLOW_VALUE_REFRESH));
    list_rpush(flows, list_node_new(FLOW_VALUE_PASSWORD));
    list_rpush(flows, list_node_new(FLOW_VALUE_CODE));
    list_rpush(flows, list_node_new(FLOW_VALUE_DEVICE));
    return flows;
  }
  flows->free = (void (*) (void*)) &clearFreeString;
  if(flow[0]!='[') {
    list_rpush(flows, list_node_new(oidc_sprintf("%s", flow)));
    return flows;
  }
  char* tmp = JSONArrrayToDelimitedString(flow, ' ');
  char* str = oidc_sprintf("%s", strtok(tmp, " "));
  list_rpush(flows, list_node_new(str));
  while((str=strtok(NULL, " "))) {
    list_rpush(flows, list_node_new(oidc_sprintf("%s", str)));
  }
  clearFreeString(tmp);

  return flows;
}

