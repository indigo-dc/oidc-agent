#include "add_handler.h"
#include "account/account.h"
#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "oidc-add/parse_ipc.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/passwords/password_entry.h"
#include "utils/prompt.h"

#include <stdlib.h>

struct resultWithEncryptionPassword {
  void* result;
  char* password;
};

struct resultWithEncryptionPassword getAccountConfigAndPassword(char* account) {
  struct oidc_account* p        = NULL;
  char*                password = NULL;
  while (NULL == p) {
    secFree(password);
    password = promptPassword(
        "Enter encryption password for account config %s: ", account);
    p = decryptAccount(account, password);
  }
  char* json_p = accountToJSONString(p);
  secFreeAccount(p);
  return (struct resultWithEncryptionPassword){.result   = json_p,
                                               .password = password};
}

char* getAccountConfig(char* account) {
  struct resultWithEncryptionPassword res =
      getAccountConfigAndPassword(account);
  secFree(res.password);
  return res.result;
}

void add_handleAdd(char* account, struct lifetimeArg lifetime) {
  struct resultWithEncryptionPassword result =
      getAccountConfigAndPassword(account);
  char* json_p   = result.result;
  char* password = result.password;

  // TODO depending on cl arguments
  struct password_entry pw = {.shortname = account, .password = password};
  pwe_setExpiresIn(&pw, lifetime.lifetime);
  pwe_setType(&pw, PW_TYPE_MEM);
  char* pw_str = passwordEntryToJSONString(&pw);
  secFree(password);

  char* res = NULL;
  if (lifetime.argProvided) {
    res = ipc_cryptCommunicate(REQUEST_ADD_LIFETIME, json_p, lifetime.lifetime,
                               pw_str);
  } else {
    res = ipc_cryptCommunicate(REQUEST_ADD, json_p, pw_str);
  }
  secFree(pw_str);
  secFree(json_p);
  add_parseResponse(res);
}

void add_assertAgent() {
  char* res = ipc_cryptCommunicate(REQUEST_CHECK);
  if (res == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  secFree(res);
}

void add_handleRemove(const char* account) {
  char* res = ipc_cryptCommunicate(REQUEST_REMOVE, account);
  add_parseResponse(res);
}

void add_handleRemoveAll() {
  char* res = ipc_cryptCommunicate(REQUEST_REMOVEALL);
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
    res = ipc_cryptCommunicate(REQUEST_LOCK, REQUEST_VALUE_UNLOCK, password);
  } else {  // locking agent
    char* passwordConfirm = promptPassword("Confirm lock password: ");
    if (!strequal(password, passwordConfirm)) {
      printError("Passwords do not match.\n");
      secFree(password);
      secFree(passwordConfirm);
      exit(EXIT_FAILURE);
    }
    secFree(passwordConfirm);
    res = ipc_cryptCommunicate(REQUEST_LOCK, REQUEST_VALUE_LOCK, password);
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
  list_mergeSort(list, (int (*)(const void*, const void*))compareFilesByName);
  char* str = listToDelimitedString(list, '\n');
  list_destroy(list);
  printf("The following account configurations are usable: \n%s\n", str);
  secFree(str);
}
