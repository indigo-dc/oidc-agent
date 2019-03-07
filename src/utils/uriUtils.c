#include "uriUtils.h"
#include "defines/agent_values.h"
#include "list/list.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <stddef.h>
#include <string.h>
#include <syslog.h>

struct codeState codeStateFromURI(const char* uri) {
  if (uri == NULL) {
    oidc_setArgNullFuncError(__func__);
    return (struct codeState){};
  }
  char* tmp       = oidc_strcopy(uri);
  char* tmp_uri   = strtok(tmp, "?");
  char* args      = strtok(NULL, "");
  char* arg1      = strtok(args, "&");
  char* arg2      = strtok(NULL, "");
  char* tmp_state = NULL;
  char* tmp_code  = NULL;
  if (strSubStringCase(arg1, "state")) {
    strtok(arg1, "=");
    tmp_state = strtok(NULL, "");
  }
  if (strSubStringCase(arg2, "state")) {
    strtok(arg2, "=");
    tmp_state = strtok(NULL, "");
  }
  if (strSubStringCase(arg1, "code")) {
    strtok(arg1, "=");
    tmp_code = strtok(NULL, "");
  }
  if (strSubStringCase(arg2, "code")) {
    strtok(arg2, "=");
    tmp_code = strtok(NULL, "");
  }
  char* state    = oidc_strcopy(tmp_state);
  char* code     = oidc_strcopy(tmp_code);
  char* base_uri = oidc_strcopy(tmp_uri);
  secFree(tmp);
  if (base_uri == NULL) {
    oidc_errno = OIDC_ENOBASEURI;
  } else if (state == NULL) {
    oidc_errno = OIDC_ENOSTATE;
  } else if (code == NULL) {
    oidc_errno = OIDC_ENOCODE;
  }
  return (struct codeState){.uri = base_uri, .state = state, .code = code};
}

void secFreeCodeState(struct codeState cs) {
  secFree(cs.code);
  secFree(cs.state);
  secFree(cs.uri);
}

char* findCustomSchemeUri(list_t* uris) {
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(uris, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* uri = node->val;
    if (strstarts(uri, AGENT_CUSTOM_SCHEME)) {
      list_iterator_destroy(it);
      return uri;
    }
  }
  list_iterator_destroy(it);
  return NULL;
}

char* extractParameterValueFromUri(const char* uri, const char* parameter) {
  if (uri == NULL || parameter == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Extracting parameter '%s' from uri '%s'",
         parameter, uri);
  char* tmp    = oidc_strcopy(uri);
  char* params = strchr(tmp, '?');
  if (params == NULL) {
    return NULL;
  }
  params++;
  char* param_k = strtok(params, "=");
  char* param_v = strtok(NULL, "&");
  char* value   = NULL;
  while (value == NULL && param_k != NULL && param_v != NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "URI contains parameter: %s - %s", param_k,
           param_v);
    if (strequal(parameter, param_k)) {
      value = oidc_strcopy(param_v);
      break;
    }
    param_k = strtok(NULL, "=");
    param_v = strtok(NULL, "&");
  }
  secFree(tmp);
  return value;
}
