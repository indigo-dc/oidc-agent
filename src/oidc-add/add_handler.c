#include "add_handler.h"
#include "account/account.h"
#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "oidc-add/parse_ipc.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/password_entry.h"
#include "utils/prompt.h"
#include "utils/system_runner.h"

#include <stdlib.h>

struct resultWithEncryptionPassword {
  void* result;
  char* password;
};

struct resultWithEncryptionPassword getAccountConfigAndPassword(
    char* account, struct arguments* arguments) {
  struct oidc_account* p        = NULL;
  char*                password = NULL;
  if (arguments->pw_cmd) {
    password = getOutputFromCommand(arguments->pw_cmd);
  };
  p = decryptAccount(account, password);

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

char* getAccountConfig(char* account, struct arguments* arguments) {
  struct resultWithEncryptionPassword res =
      getAccountConfigAndPassword(account, arguments);
  secFree(res.password);
  return res.result;
}

time_t getPWExpiresInDependingOn(struct arguments* arguments) {
  if (arguments->pw_lifetime.argProvided == ARG_PROVIDED_BUT_USES_DEFAULT &&
      arguments->lifetime.argProvided) {
    return arguments->lifetime.lifetime;
  }
  return arguments->pw_lifetime.lifetime;
}

void add_handleAdd(char* account, struct arguments* arguments) {
  struct resultWithEncryptionPassword result =
      getAccountConfigAndPassword(account, arguments);
  char* json_p   = result.result;
  char* password = result.password;

  struct password_entry pw   = {.shortname = account};
  unsigned char         type = PW_TYPE_PRMT;
  if (arguments->pw_cmd) {
    pwe_setCommand(&pw, arguments->pw_cmd);
    type |= PW_TYPE_CMD;
  }
  if (arguments->pw_lifetime.argProvided) {
    pwe_setPassword(&pw, password);
    pwe_setExpiresIn(&pw, getPWExpiresInDependingOn(arguments));
    type |= PW_TYPE_MEM;
  }
  if (arguments->pw_keyring) {
    if (!arguments->pw_lifetime
             .argProvided) {  // Only set password if not already done
      pwe_setPassword(&pw, password);
    }
    type |= PW_TYPE_MNG;
  }
  pwe_setType(&pw, type);
  char* pw_str = passwordEntryToJSONString(&pw);
  secFree(password);

  char* res = NULL;
  if (arguments->lifetime.argProvided) {
    res = ipc_cryptCommunicate(REQUEST_ADD_LIFETIME, json_p,
                               arguments->lifetime.lifetime, pw_str,
                               arguments->confirm);
  } else {
    res = ipc_cryptCommunicate(REQUEST_ADD, json_p, pw_str, arguments->confirm);
  }
  secFree(pw_str);
  secFree(json_p);
  add_parseResponse(res);
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

void add_handlePrint(char* account, struct arguments* arguments) {
  char* json_p = getAccountConfig(account, arguments);
  printf("%s\n", json_p);
  secFree(json_p);
}
