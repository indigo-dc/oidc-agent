#include "uriUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <stddef.h>
#include <string.h>

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
