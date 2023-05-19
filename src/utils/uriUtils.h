#ifndef OIDC_URIUTILS_H
#define OIDC_URIUTILS_H

#include "utils/oidc_error.h"
#include "wrapper/list.h"

struct codeState {
  char* code;
  char* state;
  char* uri;
};

struct codeState codeStateFromURI(const char* uri);
void             secFreeCodeState(struct codeState cs);
char*            findCustomSchemeUri(list_t* uris);
char* extractParameterValueFromUri(const char* uri, const char* parameter);
char* getBaseUri(const char* uri);
char* getTopHost(const char* uri);
oidc_error_t checkRedirectUrisForErrors(list_t* redirect_uris);

#endif  // OIDC_URIUTILS_H
