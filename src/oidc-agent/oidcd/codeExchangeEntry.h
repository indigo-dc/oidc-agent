#ifndef OIDC_CODEEXCHANGEENTRY_H
#define OIDC_CODEEXCHANGEENTRY_H

#include "account/account.h"

struct codeExchangeEntry {
  char*                state;
  struct oidc_account* account;
  char*                code_verifier;
};

int cee_matchByState(struct codeExchangeEntry* a, struct codeExchangeEntry* b);
struct codeExchangeEntry* createCodeExchangeEntry(char*                state,
                                                  struct oidc_account* account,
                                                  char* code_verifier);
void secFreeCodeExchangeContent(struct codeExchangeEntry* cee);

#endif  // OIDC_CODEEXCHANGEENTRY_H
