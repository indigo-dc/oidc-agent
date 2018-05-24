#include "add_handler.h"
#include "api.h"
#include "prompt.h"
#include "account.h"
#include "file_io.h"
#include "parse_ipc.h"
#include "utils/listUtils.h"

#include <stdlib.h>

char* getAccountConfig(char* account) {
  struct oidc_account* p = NULL;
  while(NULL==p) {
    char* password = promptPassword("Enter encryption password for account config %s: ", account);
    p = decryptAccount(account, password);
    clearFreeString(password);
  }
  char* json_p = accountToJSON(*p);
  clearFreeAccount(p);
  return json_p;
}

void add_handleAddAndRemove(char* account, int remove) {
  char* json_p = getAccountConfig(account);

  char* res = communicate(REQUEST_CONFIG, remove ? REQUEST_VALUE_REMOVE : REQUEST_VALUE_ADD, json_p);
  clearFreeString(json_p);
  add_parseResponse(res);
}

void add_handlePrint(char* account) {
  char* json_p = getAccountConfig(account);
  printf("%s\n", json_p);
  clearFreeString(json_p);
}

void add_handleList() {
  list_t* list = getAccountConfigFileList();
  char* str = listToDelimitedString(list, ' ');
  list_destroy(list);
  printf("The following account configurations are usable: %s\n", str); 
  clearFreeString(str);
}
