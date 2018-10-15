#include "add_handler.h"
#include "account.h"
#include "file_io/oidc_file_io.h"
#include "ipc/communicator.h"
#include "ipc/ipc_values.h"
#include "parse_ipc.h"
#include "prompt.h"
#include "utils/listUtils.h"

#include <stdlib.h>

char* getAccountConfig(char* account) {
  struct oidc_account* p = NULL;
  while (NULL == p) {
    char* password = promptPassword(
        "Enter encryption password for account config %s: ", account);
    p = decryptAccount(account, password);
    secFree(password);
  }
  char* json_p = accountToJSONString(*p);
  secFreeAccount(p);
  return json_p;
}

void add_handleAddAndRemove(char* account, int remove,
                            struct lifetimeArg lifetime) {
  char* json_p = getAccountConfig(account);

  char* res = NULL;
  if (remove) {
    res = ipc_communicate(REQUEST_CONFIG, REQUEST_VALUE_REMOVE, json_p);
  } else if (lifetime.argProvided) {
    res = ipc_communicate(REQUEST_ADD_LIFETIME, json_p, lifetime.lifetime);
  } else {
    res = ipc_communicate(REQUEST_CONFIG, REQUEST_VALUE_ADD, json_p);
  }
  secFree(json_p);
  add_parseResponse(res);
}

void add_handleLock(int lock) {
  char* password = promptPassword("Enter lock password: ");
  if (password == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  char* res = NULL;
  if (!lock) {  // unlocking agent
    res = ipc_communicate(REQUEST_LOCK, REQUEST_VALUE_UNLOCK, password);
  } else {  // locking agent
    char* passwordConfirm = promptPassword("Confirm lock password: ");
    if (!strequal(password, passwordConfirm)) {
      printError("Passwords do not match.\n");
      secFree(password);
      secFree(passwordConfirm);
      exit(EXIT_FAILURE);
    }
    secFree(passwordConfirm);
    res = ipc_communicate(REQUEST_LOCK, REQUEST_VALUE_LOCK, password);
  }
  secFree(password);
  add_parseResponse(res);
}

void add_handlePrint(char* account) {
  char* json_p = getAccountConfig(account);
  printf("%s\n", json_p);
  secFree(json_p);
}

void add_handleList() {
  list_t* list = getAccountConfigFileList();
  char*   str  = listToDelimitedString(list, ' ');
  list_destroy(list);
  printf("The following account configurations are usable: %s\n", str);
  secFree(str);
}
