#include "uriUtils.h"
#include "defines/agent_values.h"
#include "list/list.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

oidc_error_t urldecode(char* dst, const char* src) {
  if (dst == NULL || src == NULL) {
    return OIDC_EARGNULL;
  }
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
  return OIDC_SUCCESS;
}

char* getBaseUri(const char* uri) {
  if (uri == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* tmp     = oidc_strcopy(uri);
  char* tmp_uri = strtok(tmp, "?");
  char* base    = oidc_strcopy(tmp_uri);
  secFree(tmp);
  urldecode(base, base);
  return base;
}

struct codeState codeStateFromURI(const char* uri) {
  if (uri == NULL) {
    oidc_setArgNullFuncError(__func__);
    return (struct codeState){};
  }
  char* state    = extractParameterValueFromUri(uri, "state");
  char* code     = extractParameterValueFromUri(uri, "code");
  char* base_uri = getBaseUri(uri);
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
  logger(DEBUG, "Extracting parameter '%s' from uri '%s'", parameter, uri);
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
    // logger(DEBUG, "URI contains parameter: %s - %s",
    // param_k, param_v);
    if (strequal(parameter, param_k)) {
      value = oidc_strcopy(param_v);
      break;
    }
    param_k = strtok(NULL, "=");
    param_v = strtok(NULL, "&");
  }
  secFree(tmp);
  urldecode(value, value);
  logger(DEBUG, "Extracted value is '%s'", value);
  return value;
}
