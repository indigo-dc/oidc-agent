#include "codeExchangeEntry.h"

#include "utils/matcher.h"

void secFreeCodeExchangeContent(struct codeExchangeEntry* cee) {
  secFreeAccount(cee->account);
  secFree(cee->code_verifier);
  secFree(cee->state);
}
void secFreeCodeExchangeEntry(struct codeExchangeEntry* cee) {
  secFreeCodeExchangeContent(cee);
  secFree(cee);
}

struct codeExchangeEntry* createCodeExchangeEntry(char*                state,
                                                  struct oidc_account* account,
                                                  char* code_verifier) {
  struct codeExchangeEntry* cee = secAlloc(sizeof(struct codeExchangeEntry));
  cee->account                  = account;
  cee->state                    = state;
  cee->code_verifier            = code_verifier;
  return cee;
}

int cee_matchByState(struct codeExchangeEntry* a, struct codeExchangeEntry* b) {
  return matchStrings(a->state, b->state);
}
