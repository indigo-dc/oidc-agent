#ifndef OIDC_URIUTILS_H
#define OIDC_URIUTILS_H

struct codeState {
  char* code;
  char* state;
  char* uri;
};

struct codeState codeStateFromURI(const char* uri);
void             secFreeCodeState(struct codeState cs);

#endif  // OIDC_URIUTILS_H
