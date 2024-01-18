#include "add_handler.h"

#include <stdlib.h>

#include "defines/ipc_values.h"
#include "ipc/cryptCommunicator.h"
#include "oidc-add/parse_ipc.h"
#include "utils/accountUtils.h"
#include "utils/config/issuerConfig.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/password_entry.h"
#include "utils/printer.h"
#include "utils/prompting/prompt.h"
#include "utils/prompting/promptUtils.h"
#include "utils/string/stringUtils.h"

time_t getPWExpiresInDependingOn(struct arguments* arguments) {
  if (arguments->pw_lifetime.argProvided == ARG_PROVIDED_BUT_USES_DEFAULT &&
      arguments->lifetime.argProvided) {
    return arguments->lifetime.lifetime;
  }
  return arguments->pw_lifetime.lifetime;
}

unsigned char checkIfAccountIsLoaded(struct arguments* arguments,
                                     const char* const account) {
  char* res = ipc_cryptCommunicate(arguments->remote, REQUEST_LOADEDACCOUNTS);
  return add_checkLoadedAccountsResponseForAccount(res, account);
}

void add_handleAdd(char* account, struct arguments* arguments) {
  if (!arguments->force && checkIfAccountIsLoaded(arguments, account)) {
    printStdout("Account '%s' already loaded\n", account);
    exit(EXIT_SUCCESS);
  }
  struct resultWithEncryptionPassword result =
      getDecryptedAccountAsStringAndPasswordFromFilePrompt(
          account, arguments->pw_cmd, arguments->pw_file, arguments->pw_env);
  char* json_p = result.result;
  if (json_p == NULL) {
    secFree(result.password);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  char* iss = getJSONValueFromString(json_p, OIDC_KEY_ISSUER);
  if (iss == NULL) {
    iss = getJSONValueFromString(json_p, AGENT_KEY_ISSUERURL);
  }
  const struct issuerConfig* iss_c = getIssuerConfig(iss);
  secFree(iss);
  char* password = result.password;

  struct password_entry pw   = {.shortname = account};
  unsigned char         type = PW_TYPE_PRMT;
  if (arguments->pw_cmd) {
    pwe_setCommand(&pw, arguments->pw_cmd);
    type |= PW_TYPE_CMD;
  }
  unsigned char storePW =
      arguments->pw_lifetime.argProvided || (iss_c && iss_c->store_pw);
  if (storePW && password) {
    pwe_setPassword(&pw, password);
    pwe_setExpiresIn(&pw, getPWExpiresInDependingOn(arguments));
    type |= PW_TYPE_MEM;
  }
  if (arguments->pw_env) {
    if (pw.password == NULL) {
      pwe_setPassword(&pw, password);
      type |= PW_TYPE_MEM;
    }
  }
  if (arguments->pw_file) {
    pwe_setFile(&pw, arguments->pw_file);
    type |= PW_TYPE_FILE;
  }
  pwe_setType(&pw, type);
  char* pw_str = passwordEntryToJSONString(&pw);
  secFree(password);

  char* res = NULL;
  if (storePW) {
    res = ipc_cryptCommunicate(
        arguments->remote, REQUEST_ADD_LIFETIME, json_p,
        arguments->lifetime.lifetime, pw_str, arguments->confirm,
        arguments->always_allow_idtoken, arguments->plainadd);
  } else {
    res = ipc_cryptCommunicate(
        arguments->remote, REQUEST_ADD, json_p, pw_str, arguments->confirm,
        arguments->always_allow_idtoken, arguments->plainadd);
  }
  secFree(pw_str);
  secFree(json_p);
  add_parseResponse(res);
}

void add_handleRemove(const char* account, struct arguments* arguments) {
  char* res = ipc_cryptCommunicate(arguments->remote, REQUEST_REMOVE, account);
  add_parseResponse(res);
}

void add_handleRemoveAll(struct arguments* arguments) {
  char* res = ipc_cryptCommunicate(arguments->remote, REQUEST_REMOVEALL);
  add_parseResponse(res);
}

void add_handleLock(int lock, struct arguments* arguments) {
  char* password = promptPassword("Enter lock password", "Password", NULL,
                                  CLI_PROMPT_VERBOSE);
  if (password == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  char* res = NULL;
  if (!lock) {  // unlocking agent
    res = ipc_cryptCommunicate(arguments->remote, REQUEST_LOCK,
                               REQUEST_VALUE_UNLOCK, password);
  } else {  // locking agent
    char* passwordConfirm = promptPassword("Confirm lock password", "Password",
                                           NULL, CLI_PROMPT_VERBOSE);
    if (!strequal(password, passwordConfirm)) {
      printError("Passwords do not match.\n");
      secFree(password);
      secFree(passwordConfirm);
      exit(EXIT_FAILURE);
    }
    secFree(passwordConfirm);
    res = ipc_cryptCommunicate(arguments->remote, REQUEST_LOCK,
                               REQUEST_VALUE_LOCK, password);
  }
  secFree(password);
  add_parseResponse(res);
}

void add_handlePrint(char* account, struct arguments* arguments) {
  char* json_p = getDecryptedAccountAsStringFromFilePrompt(
      account, arguments->pw_cmd, arguments->pw_file, arguments->pw_env);
  if (json_p == NULL) {
    exit(EXIT_FAILURE);
  }
  printStdout("%s\n", json_p);
  secFree(json_p);
}

void add_handleListLoadedAccounts(struct arguments* arguments) {
  char* res = ipc_cryptCommunicate(arguments->remote, REQUEST_LOADEDACCOUNTS);
  add_parseLoadedAccountsResponse(res);
}