#include "add_handler.h"
#include "api.h"
#include "prompt.h"
#include "account.h"
#include "parse_ipc.h"

#include <stdlib.h>

char* getAccountConfig(char* account) {
  struct oidc_account* p = NULL;
  while(NULL==p) {
    char* password = promptPassword("Enter encryption password for account config %s: ", account);
    p = decryptAccount(account, password);
    clearFreeString(password);
  }
  char* json_p = accountToJSON(*p);
  freeAccount(p);
  return json_p;
}

void add_handleAddAndRemove(char* account, int remove) {
  char* json_p = getAccountConfig(account);

  char* res = communicate(REQUEST_CONFIG, remove ? REQUEST_VALUE_REMOVE : REQUEST_VALUE_ADD, json_p);
  clearFreeString(json_p);
  add_parseResponse(res);
}
