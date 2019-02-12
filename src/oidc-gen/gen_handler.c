#define _GNU_SOURCE

#include "gen_handler.h"
#include "account/issuer_helper.h"
#include "ipc/cryptCommunicator.h"
#include "ipc/ipc_values.h"
#include "list/list.h"
#include "oidc-agent/httpserver/termHttpserver.h"
#include "oidc-agent/oidc/device_code.h"
#include "oidc-agent/oidc/values.h"
#include "oidc-gen/parse_ipc.h"
#include "settings.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/portUtils.h"
#include "utils/printer.h"
#include "utils/prompt.h"
#include "utils/stringUtils.h"

#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

struct state {
  int doNotMergeTmpFile;
} oidc_gen_state;

void handleGen(struct oidc_account* account, const struct arguments* arguments,
               char** cryptPassPtr) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (arguments->device_authorization_endpoint) {
    issuer_setDeviceAuthorizationEndpoint(
        account_getIssuer(account),
        oidc_strcopy(arguments->device_authorization_endpoint), 1);
  }
  cJSON* flow_json = listToJSONArray(arguments->flows);
  char*  log_tmp   = jsonToString(flow_json);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "arguments flows in handleGen are '%s'",
         log_tmp);
  secFree(log_tmp);
  if (flow_json == NULL || jsonArrayIsEmpty(flow_json)) {
    list_t* redirect_uris = account_getRedirectUris(account);
    if (redirect_uris != NULL && redirect_uris->len > 0) {
      flow_json = jsonArrayAddStringValue(flow_json, FLOW_VALUE_CODE);
    }
    if (strValid(account_getRefreshToken(account))) {
      flow_json = jsonArrayAddStringValue(flow_json, FLOW_VALUE_REFRESH);
    }
    if ((strValid(account_getUsername(account)) &&
         strValid(account_getPassword(account))) ||
        flow_json == NULL || jsonArrayIsEmpty(flow_json)) {
      flow_json = jsonArrayAddStringValue(flow_json, FLOW_VALUE_PASSWORD);
    }
  }
  char* flow = jsonToString(flow_json);
  secFreeJson(flow_json);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "flows in handleGen are '%s'", flow);
  if (strcasestr(flow, FLOW_VALUE_PASSWORD) &&
      (!strValid(account_getUsername(account)) ||
       !strValid(account_getPassword(account)))) {
    promptAndSetUsername(account, arguments->flows);
    promptAndSetPassword(account, arguments->flows);
  }
  char* json = accountToJSONString(account);
  printNormal("Generating account configuration ...\n");
  char* res =
      ipc_cryptCommunicate(REQUEST_CONFIG_FLOW, REQUEST_VALUE_GEN, json, flow);
  secFree(flow);
  secFree(json);
  json = NULL;
  if (NULL == res) {
    printError("Error: %s\n", oidc_serror());
    if (cryptPassPtr) {
      secFree(*cryptPassPtr);
      secFree(cryptPassPtr);
    }
    secFreeAccount(account);
    exit(EXIT_FAILURE);
  }
  json = gen_parseResponse(res, arguments);

  char* issuer = getJSONValueFromString(json, "issuer_url");
  updateIssuerConfig(issuer);
  secFree(issuer);

  char* name = getJSONValueFromString(json, "name");
  char* hint = oidc_sprintf("account configuration '%s'", name);
  encryptAndWriteConfig(json, account_getName(account), hint,
                        cryptPassPtr ? *cryptPassPtr : NULL, NULL, name,
                        arguments->verbose);
  secFree(name);
  secFree(hint);
  secFreeAccount(account);

  if (cryptPassPtr) {
    secFree(*cryptPassPtr);
    secFree(cryptPassPtr);
  }
  secFree(json);
}

void manualGen(struct oidc_account*    account,
               const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  char** cryptPassPtr = secAlloc(sizeof(char*));
  account             = genNewAccount(account, arguments, cryptPassPtr);
  handleGen(account, arguments, cryptPassPtr);
}

void handleCodeExchange(const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  int   needFree   = 0;
  char* short_name = arguments->args[0];
  while (!strValid(short_name)) {
    if (needFree) {
      secFree(short_name);
    }
    short_name = prompt("Enter short name for the account to configure: ");
    needFree   = 1;
  }

  char* res = ipc_cryptCommunicate(arguments->codeExchangeRequest);
  if (NULL == res) {
    printError("Error: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
    if (needFree) {
      secFree(short_name);
    }
  }
  char* config = gen_parseResponse(res, arguments);

  char* hint = oidc_sprintf("account configuration '%s'", short_name);
  encryptAndWriteConfig(config, short_name, hint, NULL, NULL, short_name,
                        arguments->verbose);
  secFree(hint);
  if (needFree) {
    secFree(short_name);
  }
  secFree(config);
}

void handleStateLookUp(const char* state, const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  char* res    = NULL;
  char* config = NULL;
  fprintf(stdout,
          "Polling oidc-agent to get the generated account configuration ...");
  fflush(stdout);
  int i = 0;
  while (config == NULL && i < MAX_POLL) {
    i++;
    res = ipc_cryptCommunicate(REQUEST_STATELOOKUP, state);
    if (NULL == res) {
      printf("\n");
      printError("Error: %s\n", oidc_serror());
      exit(EXIT_FAILURE);
    }
    if (arguments->verbose) {
      printNormal("%s\n", res);
    }
    config = gen_parseResponse(res, arguments);
    if (config == NULL) {
      usleep(DELTA_POLL * 1000);
      printf(".");
      fflush(stdout);
    }
  }
  printf("\n");
  if (config == NULL) {
    printNormal("Polling is boring. Already tried %d times. I stop now.\n", i);
    printImportant("Please press Enter to try it again.\n");
    getchar();
    res = ipc_cryptCommunicate(REQUEST_STATELOOKUP, state);
    if (res == NULL) {
      printError("Error: %s\n", oidc_serror());
      _secFree(ipc_cryptCommunicate(REQUEST_TERMHTTP, state));
      exit(EXIT_FAILURE);
    }
    config = gen_parseResponse(res, arguments);
    if (config == NULL) {
      printError("Could not receive generated account configuration for "
                 "state='%s'\n",
                 state);
      printImportant(
          "Please try state lookup again by using:\noidc-gen --state=%s\n",
          state);
      _secFree(ipc_cryptCommunicate(REQUEST_TERMHTTP, state));
      exit(EXIT_FAILURE);
    }
  }
  unregisterSignalHandler();
  char* issuer = getJSONValueFromString(config, "issuer_url");
  updateIssuerConfig(issuer);
  secFree(issuer);

  char* short_name = getJSONValueFromString(config, "name");
  char* hint       = oidc_sprintf("account configuration '%s'", short_name);
  encryptAndWriteConfig(config, short_name, hint, NULL, NULL, short_name,
                        arguments->verbose);
  secFree(hint);
  secFree(short_name);
  secFree(config);
  exit(EXIT_SUCCESS);
}

char* gen_handleDeviceFlow(char* json_device, char* json_account,
                           const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  struct oidc_device_code* dc = getDeviceCodeFromJSON(json_device);
  printDeviceCode(*dc, arguments->qr, arguments->qrterminal);
  size_t interval   = oidc_device_getInterval(*dc);
  size_t expires_in = oidc_device_getExpiresIn(*dc);
  long   expires_at = time(NULL) + expires_in;
  secFreeDeviceCode(dc);
  while (expires_in ? expires_at > time(NULL) : 1) {
    sleep(interval);
    char* res = ipc_cryptCommunicate(REQUEST_DEVICE, json_device, json_account);
    struct key_value pairs[3];
    pairs[0].key = "status";
    pairs[1].key = "error";
    pairs[2].key = "config";
    if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) <
        0) {
      printError("Could not decode json: %s\n", res);
      printError("This seems to be a bug. Please hand in a bug report.\n");
      secFree(res);
      exit(EXIT_FAILURE);
    }
    secFree(res);
    char* error = pairs[1].value;
    if (error) {
      if (strcmp(error, OIDC_SLOW_DOWN) == 0) {
        interval++;
        secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
        continue;
      }
      if (strcmp(error, OIDC_AUTHORIZATION_PENDING) == 0) {
        secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
        continue;
      }
      printError(error);
      secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
      exit(EXIT_FAILURE);
    }
    secFree(pairs[0].value);
    return pairs[2].value;
  }
  printError("Device code is not valid any more!");
  exit(EXIT_FAILURE);
}

struct oidc_account* genNewAccount(struct oidc_account*    account,
                                   const struct arguments* arguments,
                                   char**                  cryptPassPtr) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (account == NULL) {
    account = secAlloc(sizeof(struct oidc_account));
  }
  promptAndSetName(account, arguments->args[0], NULL);
  char* encryptionPassword = NULL;
  char* shortname          = account_getName(account);
  if (oidcFileDoesExist(shortname)) {
    struct oidc_account* loaded_p = NULL;
    unsigned int         i;
    for (i = 0; i < MAX_PASS_TRIES && NULL == loaded_p; i++) {
      secFree(encryptionPassword);
      char* prompt = oidc_sprintf(
          "Enter encryption Password for account config '%s': ", shortname);
      encryptionPassword = promptPassword(prompt);
      secFree(prompt);
      loaded_p = decryptAccount(shortname, encryptionPassword);
      if (loaded_p == NULL) {
        oidc_perror();
      }
    }
    if (loaded_p == NULL) {
      secFree(encryptionPassword);
      exit(EXIT_FAILURE);
    }
    secFreeAccount(account);
    account = loaded_p;
  } else {
    printf("No account exists with this short name. Creating new configuration "
           "...\n");
    char* tmpFile = oidc_strcat(CLIENT_TMP_PREFIX, shortname);
    if (fileDoesExist(tmpFile)) {
      if (promptConsentDefaultYes("Found temporary file for this shortname. Do "
                                  "you want to use it?")) {
        secFreeAccount(account);
        account = accountFromFile(tmpFile);
      } else {
        oidc_gen_state.doNotMergeTmpFile = 1;
      }
    }
    secFree(tmpFile);
  }
  promptAndSetCertPath(account, arguments->cert_path);
  promptAndSetIssuer(account);
  promptAndSetClientId(account);
  promptAndSetClientSecret(account, arguments->usePublicClient);
  promptAndSetScope(account);
  promptAndSetRefreshToken(account, arguments->refresh_token);
  promptAndSetUsername(account, arguments->flows);
  promptAndSetPassword(account, arguments->flows);
  promptAndSetRedirectUris(
      account, arguments->flows && strcmp(list_at(arguments->flows, 0)->val,
                                          FLOW_VALUE_DEVICE) == 0);
  *cryptPassPtr = encryptionPassword;
  return account;
}

struct oidc_account* registerClient(struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  struct oidc_account* account = secAlloc(sizeof(struct oidc_account));
  promptAndSetName(account, arguments->args[0], arguments->client_name_id);
  if (oidcFileDoesExist(account_getName(account))) {
    printError("An account with that shortname is already configured\n");
    exit(EXIT_FAILURE);
  }

  char* tmpFile = oidc_strcat(CLIENT_TMP_PREFIX, account_getName(account));
  if (fileDoesExist(tmpFile) && !arguments->usePublicClient) {
    if (promptConsentDefaultYes("Found temporary file for this shortname. Do "
                                "you want to use it?")) {
      secFreeAccount(account);
      account = accountFromFile(tmpFile);
      handleGen(account, arguments, NULL);
      exit(EXIT_SUCCESS);
    } else {
      oidc_gen_state.doNotMergeTmpFile = 1;
    }
  }
  secFree(tmpFile);

  promptAndSetCertPath(account, arguments->cert_path);
  promptAndSetIssuer(account);
  if (arguments->device_authorization_endpoint) {
    issuer_setDeviceAuthorizationEndpoint(
        account_getIssuer(account),
        oidc_strcopy(arguments->device_authorization_endpoint), 1);
  }
  promptAndSetScope(account);

  if (arguments->usePublicClient) {
    oidc_error_t pubError = gen_handlePublicClient(account, arguments);
    switch (pubError) {
      case OIDC_SUCCESS:
        // Actually we should already have exited
        exit(EXIT_SUCCESS);
      case OIDC_ENOPUBCLIENT:
        printError("Could not find a public client for this issuer.\n");
        break;
      default: oidc_perror();
    }
    exit(EXIT_FAILURE);
  }

  char* authorization = NULL;
  if (arguments->dynRegToken.useIt) {
    if (arguments->dynRegToken.str) {
      authorization = arguments->dynRegToken.str;
    } else {
      authorization =
          prompt("Registration endpoint authorization access token: ");
    }
  }
  if (arguments->redirect_uris) {
    account_setRedirectUris(
        account, arguments->redirect_uris);  // Note that this will eventually
                                             // free arguments->redirect_uris; so
                                             // it should not be used afterwards
  }

  char* json = accountToJSONString(account);
  printf("Registering Client ...\n");
  char* flows = listToJSONArrayString(arguments->flows);
  char* res   = ipc_cryptCommunicate(REQUEST_REGISTER_AUTH, json, flows,
                                   authorization ?: "");
  secFree(flows);
  secFree(json);
  if (arguments->dynRegToken.useIt && arguments->dynRegToken.str == NULL) {
    secFree(authorization);
  }
  if (NULL == res) {
    printError("Error: %s\n", oidc_serror());
    secFreeAccount(account);
    exit(EXIT_FAILURE);
  }
  if (arguments->verbose && res) {
    printNormal("%s\n", res);
  }
  struct key_value pairs[4];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "client";
  pairs[3].key = "info";
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  secFree(res);
  if (pairs[1].value) {
    if (strValid(pairs[2].value)) {  // if a client was registered, but there's
                                     // still an
      // error (i.e. not all required scopes could be
      // registered) temporarily save the client config
      cJSON* json_config = stringToJson(pairs[2].value);
      jsonAddStringValue(json_config, "issuer_url",
                         account_getIssuerUrl(account));
      jsonAddStringValue(json_config, "cert_path",
                         account_getCertPath(account));
      secFree(pairs[2].value);
      char* config = jsonToString(json_config);
      secFreeJson(json_config);
      if (arguments->splitConfigFiles) {
        if (arguments->output) {
          printNormal("Writing client config to file '%s'\n",
                      arguments->output);
          encryptAndWriteConfig(config, account_getName(account),
                                "client config file", NULL, arguments->output,
                                NULL, arguments->verbose);
        } else {
          char* client_id = getJSONValueFromString(config, "client_id");
          char* path = createClientConfigFileName(account_getIssuerUrl(account),
                                                  client_id);
          secFree(client_id);
          char* oidcdir = getOidcDir();
          printNormal("Writing client config to file '%s%s'\n", oidcdir, path);
          secFree(oidcdir);
          encryptAndWriteConfig(config, account_getName(account),
                                "client config file", NULL, NULL, path,
                                arguments->verbose);
          secFree(path);
        }
      } else {  // not splitting config files
        char* path = oidc_strcat(CLIENT_TMP_PREFIX, account_getName(account));
        if (arguments->verbose) {
          printNormal("Writing client config temporary to file '%s'\n", path);
        }
        writeFile(path, config);
        oidc_gen_state.doNotMergeTmpFile = 0;
        secFree(path);
      }
    }

    // if dyn reg not possible try using a preregistered public client
    if (errorMessageIsForError(pairs[1].value, OIDC_ENOSUPREG)) {
      printNormal("Dynamic client registration not supported by this "
                  "issuer.\nTry using a public client ...\n");
    } else {
      printNormal("The following error occured during dynamic client "
                  "registration:\n%s\n",
                  pairs[1].value);
      if (pairs[3].value) {
        printNormal("%s\n", pairs[3].value);
      }
      printNormal("Try using a public client ...\n");
    }
    oidc_error_t pubError = gen_handlePublicClient(account, arguments);
    switch (pubError) {
      case OIDC_SUCCESS:
        // Actually we should already have exited
        exit(EXIT_SUCCESS);
      case OIDC_ENOPUBCLIENT:
        printError(
            "Dynamic client registration not successfull for this issuer "
            "and could not find a public client for this issuer.\n");
        break;
      default: oidc_perror();
    }
    printIssuerHelp(account_getIssuerUrl(account));
    secFreeAccount(account);
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    exit(EXIT_FAILURE);
  }
  if (pairs[3].value) {
    printImportant("%s\n", pairs[3].value);
    secFree(pairs[3].value);
  }
  secFree(pairs[0].value);
  secFree(pairs[1].value);
  if (pairs[2].value) {
    char*  client_config      = pairs[2].value;
    cJSON* client_config_json = stringToJson(client_config);
    secFree(client_config);
    cJSON* account_config_json = accountToJSONWithoutCredentials(account);
    cJSON* merged_json =
        mergeJSONObjects(client_config_json, account_config_json);
    secFreeJson(account_config_json);
    secFreeJson(client_config_json);
    char* new_scope_value = getJSONValue(merged_json, "scope");
    if (!strSubStringCase(new_scope_value, OIDC_SCOPE_OPENID) ||
        !strSubStringCase(new_scope_value, OIDC_SCOPE_OFFLINE_ACCESS)) {
      printError("Registered client does not have all the required scopes: %s "
                 "%s\nPlease contact the provider to update the client to have "
                 "all the requested scope values.\n",
                 OIDC_SCOPE_OPENID, OIDC_SCOPE_OFFLINE_ACCESS);
      printIssuerHelp(account_getIssuerUrl(account));
      secFree(new_scope_value);
      secFreeJson(merged_json);
      exit(EXIT_FAILURE);
    }
    secFree(new_scope_value);
    char* text = jsonToString(merged_json);
    secFreeJson(merged_json);
    if (text == NULL) {
      oidc_perror();
      exit(EXIT_FAILURE);
    }
    if (arguments->splitConfigFiles) {
      if (arguments->output) {
        printImportant("Writing client config to file '%s'\n",
                       arguments->output);
        encryptAndWriteConfig(text, account_getName(account),
                              "client config file", NULL, arguments->output,
                              NULL, arguments->verbose);
      } else {
        char* client_id = getJSONValueFromString(text, "client_id");
        char* path = createClientConfigFileName(account_getIssuerUrl(account),
                                                client_id);
        secFree(client_id);
        char* oidcdir = getOidcDir();
        printImportant("Writing client config to file '%s%s'\n", oidcdir, path);
        secFree(oidcdir);
        encryptAndWriteConfig(text, account_getName(account),
                              "client config file", NULL, NULL, path,
                              arguments->verbose);
        secFree(path);
      }
    } else {  // not splitting config files
      char* path = oidc_strcat(CLIENT_TMP_PREFIX, account_getName(account));
      if (arguments->verbose) {
        printf("Writing client config temporary to file '%s'\n", path);
      }
      writeFile(path, text);
      oidc_gen_state.doNotMergeTmpFile = 0;
      secFree(path);
    }
    struct oidc_account* updatedAccount = getAccountFromJSON(text);
    secFree(text);
    // account_setIssuerUrl(updatedAccount,
    //                      oidc_strcopy(account_getIssuerUrl(*account)));
    // account_setName(updatedAccount, oidc_strcopy(account_getName(*account)),
    //                 NULL);
    // account_setClientName(updatedAccount,
    //                       oidc_strcopy(account_getClientName(*account)));
    // account_setCertPath(updatedAccount,
    //                     oidc_strcopy(account_getCertPath(*account)));
    secFreeAccount(account);
    return updatedAccount;
  }
  secFree(pairs[2].value);
  secFreeAccount(account);
  return NULL;
}

void handleDelete(const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (!oidcFileDoesExist(arguments->args[0])) {
    printError("No account with that shortname configured\n");
    exit(EXIT_FAILURE);
  }
  struct oidc_account* loaded_p           = NULL;
  char*                encryptionPassword = NULL;
  unsigned int         i;
  for (i = 0; i < MAX_PASS_TRIES && NULL == loaded_p; i++) {
    char* forWhat = oidc_sprintf("account config '%s'", arguments->args[0]);
    encryptionPassword =
        getEncryptionPassword(forWhat, NULL, MAX_PASS_TRIES - i);
    secFree(forWhat);
    loaded_p = decryptAccount(arguments->args[0], encryptionPassword);
    secFree(encryptionPassword);
    if (loaded_p == NULL) {
      oidc_perror();
    }
  }
  if (loaded_p == NULL) {
    return;
  }
  char* json = accountToJSONString(loaded_p);
  secFreeAccount(loaded_p);
  deleteClient(arguments->args[0], json, 1);
  secFree(json);
}

void deleteClient(char* short_name, char* account_json, int revoke) {
  char* res = ipc_cryptCommunicate(revoke ? REQUEST_DELETE : REQUEST_REMOVE,
                                   revoke ? account_json : short_name);

  struct key_value pairs[2];
  pairs[0].key = "status";
  pairs[1].key = "error";
  if (getJSONValuesFromString(res, pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  secFree(res);
  if (strcmp(pairs[0].value, STATUS_SUCCESS) == 0 ||
      strcmp(pairs[1].value, ACCOUNT_NOT_LOADED) == 0) {
    printf("The generated account was successfully removed from oidc-agent. "
           "You don't have to run oidc-add.\n");
    secFree(pairs[0].value);
    if (removeOidcFile(short_name) == 0) {
      printf("Successfully deleted account configuration.\n");
    } else {
      printError("error removing configuration file: %s", oidc_serror());
    }

    exit(EXIT_SUCCESS);
  }
  if (pairs[1].value != NULL) {
    printError("Error: %s\n", pairs[1].value);
    if (strstarts(pairs[1].value, "Could not revoke token:")) {
      if (promptConsentDefaultNo(
              "Do you want to unload and delete anyway. You then have to "
              "revoke the refresh token manually.")) {
        deleteClient(short_name, account_json, 0);
      } else {
        printError(
            "The account was not removed from oidc-agent due to the above "
            "listed error. You can fix the error and try it again.\n");
      }
    } else {
      printError("The account was not removed from oidc-agent due to the above "
                 "listed error. You can fix the error and try it again.\n");
    }
    secFree(pairs[1].value);
    secFree(pairs[0].value);
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief creates account from config file.
 * The config file is provided by the user. It might be a clientconfig file
 * created and encrypted by oidc-gen or an unencrypted file.
 * @param filename the absolute path of the account config file
 * @return a pointer to the result oidc_account struct. Has to be freed after
 * usage using \f secFree
 */
struct oidc_account* accountFromFile(const char* filename) {
  char* inputconfig = readFile(filename);
  if (!inputconfig) {
    printError("Could not read config file: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Read config from user provided file: %s",
         inputconfig);
  struct oidc_account* account = getAccountFromJSON(inputconfig);
  if (!account) {
    char* encryptionPassword = NULL;
    int   i;
    for (i = 0; i < MAX_PASS_TRIES && account == NULL; i++) {
      syslog(LOG_AUTHPRIV | LOG_DEBUG,
             "Read config from user provided file: %s", inputconfig);
      encryptionPassword =
          promptPassword("Enter decryption Password for client config file: ");
      account = decryptAccountText(inputconfig, encryptionPassword);
      secFree(encryptionPassword);
      if (account == NULL) {
        oidc_perror();
      }
    }
  }
  secFree(inputconfig);
  return account;
}

/**
 * @brief updates the issuer.config file.
 * If the issuer url is not already in the issuer.config file, it will be added.
 * @param issuer_url the issuer url to be added
 */
void updateIssuerConfig(const char* issuer_url) {
  char* issuers = readOidcFile(ISSUER_CONFIG_FILENAME);
  char* new_issuers;
  if (issuers) {
    if (strcasestr(issuers, issuer_url) != NULL) {
      secFree(issuers);
      return;
    }
    new_issuers = oidc_sprintf("%s\n%s", issuers, issuer_url);
    secFree(issuers);
  } else {
    new_issuers = oidc_strcopy(issuer_url);
  }
  if (new_issuers == NULL) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "%s", oidc_serror());
  } else {
    writeOidcFile(ISSUER_CONFIG_FILENAME, new_issuers);
    secFree(new_issuers);
  }
}

/**
 * @brief encrypts and writes an account configuration.
 * @param config the json encoded account configuration text. Might be
 * merged with a client configuration file
 * @param shortname the account short name
 * @param suggestedPassword the suggestedPassword for encryption, won't be
 * displayed; can be NULL.
 * @param filepath an absolute path to the output file. Either filepath or
 * filename has to be given. The other one shall be NULL.
 * @param filename the filename of the output file. The output file will be
 * placed in the oidc dir. Either filepath or filename has to be given. The
 * other one shall be NULL.
 * @return an oidc_error code. oidc_errno is set properly.
 */
oidc_error_t encryptAndWriteConfig(const char* config, const char* shortname,
                                   const char* hint,
                                   const char* suggestedPassword,
                                   const char* filepath,
                                   const char* oidc_filename, int verbose) {
  char* tmpFile = oidc_strcat(CLIENT_TMP_PREFIX, shortname);
  if (oidc_gen_state.doNotMergeTmpFile || !fileDoesExist(tmpFile)) {
    secFree(tmpFile);
    if (verbose) {
      printf("The following data will be saved encrypted:\n%s\n", config);
    }
    return encryptAndWriteText(config, hint, suggestedPassword, filepath,
                               oidc_filename);
  }
  char* tmpcontent = readFile(tmpFile);
  char* text       = mergeJSONObjectStrings(config, tmpcontent);
  secFree(tmpcontent);
  if (text == NULL) {
    secFree(tmpFile);
    oidc_perror();
    return oidc_errno;
  }
  if (verbose) {
    printf("The following data will be saved encrypted:\n%s\n", text);
  }
  oidc_error_t e = encryptAndWriteText(text, hint, suggestedPassword, filepath,
                                       oidc_filename);
  secFree(text);
  if (e == OIDC_SUCCESS) {
    removeFile(tmpFile);
  }
  secFree(tmpFile);
  return e;
}

/**
 * @brief encrypts and writes a given text.
 * @param text the json encoded account configuration text
 * @param suggestedPassword the suggestedPassword for encryption, won't be
 * displayed; can be NULL.
 * @param filepath an absolute path to the output file. Either filepath or
 * filename has to be given. The other one shall be NULL.
 * @param filename the filename of the output file. The output file will be
 * placed in the oidc dir. Either filepath or filename has to be given. The
 * other one shall be NULL.
 * @return an oidc_error code. oidc_errno is set properly.
 */
oidc_error_t encryptAndWriteText(const char* text, const char* hint,
                                 const char* suggestedPassword,
                                 const char* filepath,
                                 const char* oidc_filename) {
  initCrypt();
  char* encryptionPassword =
      getEncryptionPassword(hint, suggestedPassword, UINT_MAX);
  if (encryptionPassword == NULL) {
    return oidc_errno;
  }
  oidc_error_t ret = encryptAndWriteWithPassword(text, encryptionPassword,
                                                 filepath, oidc_filename);
  secFree(encryptionPassword);
  return ret;
}

/**
 * @brief encrypts and writes a given text with the given password.
 * @param text the text to be encrypted
 * @param password the encryption password
 * @param filepath an absolute path to the output file. Either filepath or
 * filename has to be given. The other one shall be NULL.
 * @param filename the filename of the output file. The output file will be
 * placed in the oidc dir. Either filepath or filename has to be given. The
 * other one shall be NULL.
 * @return an oidc_error code. oidc_errno is set properly.
 */
oidc_error_t encryptAndWriteWithPassword(const char* text, const char* password,
                                         const char* filepath,
                                         const char* oidc_filename) {
  if (text == NULL || password == NULL ||
      (filepath == NULL && oidc_filename == NULL)) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* toWrite = encryptWithVersionLine(text, password);
  if (toWrite == NULL) {
    return oidc_errno;
  }
  if (filepath) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Write to file %s", filepath);
    writeFile(filepath, toWrite);
  } else {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Write to oidc file %s", oidc_filename);
    writeOidcFile(oidc_filename, toWrite);
  }
  secFree(toWrite);
  return OIDC_SUCCESS;
}

/**
 * @brief prompts the user and sets the account field using the provided
 * function.
 * @param account the account struct that will be updated
 * @param prompt_str the string used for prompting
 * @param set_callback the callback function for setting the account field
 * @param get_callback the callback function for getting the account field
 * @param passPrompt indicates if if prompting for a password. If 0 non password
 * prompt is used.
 * @param optional indicating if the field is optional or not.
 */
void promptAndSet(struct oidc_account* account, char* prompt_str,
                  void (*set_callback)(struct oidc_account*, char*),
                  char* (*get_callback)(const struct oidc_account*),
                  int passPrompt, int optional) {
  char* input = NULL;
  do {
    if (passPrompt) {
      input = promptPassword(prompt_str,
                             strValid(get_callback(account)) ? " [***]" : "");
    } else {
      input =
          prompt(prompt_str, strValid(get_callback(account)) ? " [" : "",
                 strValid(get_callback(account)) ? get_callback(account) : "",
                 strValid(get_callback(account)) ? "]" : "");
    }
    if (strValid(input)) {
      set_callback(account, input);
    } else {
      secFree(input);
    }
    if (optional) {
      break;
    }
  } while (!strValid(get_callback(account)));
}

void promptAndSetIssuer(struct oidc_account* account) {
  if (!oidcFileDoesExist(ISSUER_CONFIG_FILENAME) &&
      !fileDoesExist(ETC_ISSUER_CONFIG_FILE)) {
    promptAndSet(account, "Issuer%s%s%s: ", account_setIssuerUrl,
                 account_getIssuerUrl, 0, 0);
  } else {
    useSuggestedIssuer(account);
  }
  stringifyIssuerUrl(account);
}

void promptAndSetClientId(struct oidc_account* account) {
  promptAndSet(account, "Client_id%s%s%s: ", account_setClientId,
               account_getClientId, 0, 0);
}

void promptAndSetClientSecret(struct oidc_account* account, int usePubclient) {
  promptAndSet(account, "Client_secret%s: ", account_setClientSecret,
               account_getClientSecret, 1, usePubclient);
}

void promptAndSetScope(struct oidc_account* account) {
  if (!strValid(account_getScope(account))) {
    account_setScopeExact(account, oidc_strcopy(DEFAULT_SCOPE));
  }
  promptAndSet(account,
               "Space delimited list of scopes%s%s%s: ", account_setScopeExact,
               account_getScope, 0, 0);
}

void promptAndSetRefreshToken(struct oidc_account* account,
                              struct optional_arg  refresh_token) {
  if (refresh_token.useIt) {
    if (refresh_token.str) {
      account_setRefreshToken(account, oidc_strcopy(refresh_token.str));
      return;
    }
    promptAndSet(account, "Refresh token%s%s%s: ", account_setRefreshToken,
                 account_getRefreshToken, 0, 0);
  }
}

short checkRedirectUrisForErrors(list_t* redirect_uris) {
  if (redirect_uris == NULL || redirect_uris->len == 0) {
    return 0;
  }
  short            err = 0;
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(redirect_uris, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    unsigned int port = getPortFromUri(node->val);
    if (port == 0) {
      printError("%s is not a valid redirect_uri. The redirect uri has to "
                 "be in the following format: http://localhost:<port>[/*]\n",
                 (char*)node->val);
      err = 1;
    } else if (port < MIN_PORT || port > MAX_PORT) {
      printError("The port number has to be between %d and %d\n", MIN_PORT,
                 MAX_PORT);
      err = 1;
    }
  }
  list_iterator_destroy(it);
  return err;
}

void promptAndSetRedirectUris(struct oidc_account* account, int useDevice) {
  char* input   = NULL;
  char* arr_str = listToDelimitedString(account_getRedirectUris(account), ' ');
  short err;
  do {
    input = prompt("Space separated redirect_uris [%s]: ",
                   strValid(arr_str) ? arr_str : "");
    if (strValid(input)) {
      list_t* redirect_uris = delimitedStringToList(input, ' ');
      secFree(input);
      err = checkRedirectUrisForErrors(redirect_uris);
      if (err) {
        list_destroy(redirect_uris);
        continue;
      }

      account_setRedirectUris(account, redirect_uris);
    } else {
      secFree(input);
      err = checkRedirectUrisForErrors(account_getRedirectUris(account));
      if (err) {
        account_setRedirectUris(account, NULL);
        secFree(arr_str);
        arr_str = oidc_strcopy("");
        continue;
      }
    }
    if (account_refreshTokenIsValid(account) ||
        (strValid(account_getUsername(account)) &&
         strValid(account_getPassword(account))) ||
        useDevice) {
      break;  // redirect_uris only required if no refresh token and no user
              // credentials provided
    }
    secFree(arr_str);
    arr_str = listToDelimitedString(account_getRedirectUris(account), ' ');
  } while (!strValid(arr_str) || err);
  secFree(arr_str);
}

void promptAndSetUsername(struct oidc_account* account, list_t* flows) {
  if (findInList(flows, "password")) {
    promptAndSet(account, "Username%s%s%s: ", account_setUsername,
                 account_getUsername, 0, 0);
  }
}

void promptAndSetPassword(struct oidc_account* account, list_t* flows) {
  if (findInList(flows, "password")) {
    promptAndSet(account, "Password%s: ", account_setPassword,
                 account_getPassword, 1, 0);
  }
}

void promptAndSetCertPath(struct oidc_account* account,
                          struct optional_arg  cert_path) {
  if (cert_path.useIt && cert_path.str) {
    account_setCertPath(account, oidc_strcopy(cert_path.str));
    return;
  }
  if (!strValid(account_getCertPath(account))) {
    unsigned int i;
    for (i = 0; i < sizeof(possibleCertFiles) / sizeof(*possibleCertFiles);
         i++) {
      if (fileDoesExist(possibleCertFiles[i])) {
        account_setCertPath(account, oidc_strcopy(possibleCertFiles[i]));
        break;
      }
    }
  }
  if (cert_path.useIt) {
    promptAndSet(account, "Cert Path%s%s%s: ", account_setCertPath,
                 account_getCertPath, 0, 0);
  }
}

void promptAndSetName(struct oidc_account* account, const char* short_name,
                      char* client_name_id) {
  if (short_name) {
    char* name = oidc_strcopy(short_name);
    account_setName(account, name,
                    client_name_id ? oidc_strcopy(client_name_id) : NULL);
  } else {
    char* shortname = NULL;
    while (!strValid(shortname)) {
      secFree(shortname);
      shortname = prompt("Enter short name for the account to configure: ");
    }
    char* client_identifier = NULL;
    if (client_name_id) {
      client_identifier = oidc_strcopy(client_name_id);
    } else {
      client_identifier =
          prompt("Enter optional additional client-name-identifier []: ");
    }
    account_setName(account, shortname, client_identifier);
    secFree(client_identifier);
  }
}

void useSuggestedIssuer(struct oidc_account* account) {
  list_t* issuers = getSuggestableIssuers();
  char*   fav     = getFavIssuer(account, issuers);
  printSuggestIssuer(issuers);
  int i;
  while ((i = promptIssuer(account, fav)) >= (int)issuers->len) {
    printError("input out of bound\n");
  }
  if (i >= 0) {
    struct oidc_issuer* issuer = secAlloc(sizeof(struct oidc_issuer));
    issuer_setIssuerUrl(issuer, oidc_strcopy(list_at(issuers, i)->val));
    account_setIssuer(account, issuer);
  }
  list_destroy(issuers);
}

int promptIssuer(struct oidc_account* account, const char* fav) {
  char* input = prompt("Issuer [%s]: ", fav);
  if (!strValid(input)) {
    char* iss = oidc_strcopy(fav);
    secFree(input);
    struct oidc_issuer* issuer = secAlloc(sizeof(struct oidc_issuer));
    issuer_setIssuerUrl(issuer, iss);
    account_setIssuer(account, issuer);
    return -1;
  } else if (isdigit(*input)) {
    int i = strToInt(input);
    secFree(input);
    i--;  // printed indices starts at 1 for non nerds
    return i;
  } else {
    struct oidc_issuer* issuer = secAlloc(sizeof(struct oidc_issuer));
    issuer_setIssuerUrl(issuer, input);
    account_setIssuer(account, issuer);
    return -1;
  }
}

void stringifyIssuerUrl(struct oidc_account* account) {
  int issuer_len = strlen(account_getIssuerUrl(account));
  if (account_getIssuerUrl(account)[issuer_len - 1] != '/') {
    account_setIssuerUrl(account,
                         oidc_strcat(account_getIssuerUrl(account), "/"));
  }
}

char* getEncryptionPassword(const char* forWhat, const char* suggestedPassword,
                            unsigned int max_pass_tries) {
  char*        encryptionPassword = NULL;
  unsigned int i;
  unsigned int max_tries =
      max_pass_tries == 0 ? MAX_PASS_TRIES : max_pass_tries;
  for (i = 0; i < max_tries; i++) {
    char* input =
        promptPassword("Enter encryption password for %s%s: ", forWhat,
                       strValid(suggestedPassword) ? " [***]" : "");
    if (suggestedPassword &&
        !strValid(input)) {  // use same encryption password
      secFree(input);
      encryptionPassword = oidc_strcopy(suggestedPassword);
      return encryptionPassword;
    } else {
      encryptionPassword = input;
      char* confirm      = promptPassword("Confirm encryption Password: ");
      if (strcmp(encryptionPassword, confirm) != 0) {
        printError("Encryption passwords did not match.\n");
        secFree(confirm);
        secFree(encryptionPassword);
      } else {
        secFree(confirm);
        return encryptionPassword;
      }
    }
  }
  if (encryptionPassword) {
    secFree(encryptionPassword);
  }

  oidc_errno = OIDC_EMAXTRIES;
  return NULL;
}

char* createClientConfigFileName(const char* issuer_url,
                                 const char* client_id) {
  char* path_fmt    = "%s_%s_%s.clientconfig";
  char* iss         = oidc_strcopy(issuer_url + 8);
  char* iss_new_end = strchr(iss, '/');  // cut after the first '/'
  *iss_new_end      = 0;
  char* today       = getDateString();
  char* path        = oidc_sprintf(path_fmt, iss, today, client_id);
  secFree(today);
  secFree(iss);

  if (oidcFileDoesExist(path)) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG,
           "The clientconfig file already exists. Changing path.");
    int   i       = 0;
    char* newName = NULL;
    do {
      secFree(newName);
      newName = oidc_sprintf("%s%d", path, i);
      i++;
    } while (oidcFileDoesExist(newName));
    secFree(path);
    path = newName;
  }
  return path;
}

void gen_handleList() {
  list_t* list = getClientConfigFileList();
  char*   str  = listToDelimitedString(list, '\n');
  list_destroy(list);
  printf("The following client configuration files are usable:\n%s\n", str);
  secFree(str);
}

void add_handleList() {
  list_t* list = getAccountConfigFileList();
  list_mergeSort(list, (int (*)(const void*, const void*))compareFilesByName);
  char* str = listToDelimitedString(list, '\n');
  list_destroy(list);
  printf("The following account configurations are usable: \n%s\n", str);
  secFree(str);
}

void gen_handlePrint(const char* file) {
  if (file == NULL || strlen(file) < 1) {
    printError("FILE not specified\n");
  }
  char* fileContent = NULL;
  if (file[0] == '/' || file[0] == '~') {  // absolut path
    fileContent = readFile(file);
  } else {  // file placed in oidc-dir
    fileContent = readOidcFile(file);
  }
  if (fileContent == NULL) {
    printError("Could not read file '%s'\n", file);
    exit(EXIT_FAILURE);
  }
  char* password  = NULL;
  char* decrypted = NULL;
  int   i;
  for (i = 0; i < MAX_PASS_TRIES && decrypted == NULL; i++) {
    password =
        promptPassword("Enter decryption Password for the passed file: ");
    decrypted = decryptFileContent(fileContent, password);
    secFree(password);
    if (decrypted == NULL) {
      oidc_perror();
    }
  }
  secFree(fileContent);
  if (decrypted == NULL) {
    exit(EXIT_FAILURE);
  }
  printf("%s\n", decrypted);
  secFree(decrypted);
}

char*          global_state = NULL;
__sighandler_t old_sigint;

void gen_http_signal_handler(int signo) {
  switch (signo) {
    case SIGINT:
      if (global_state) {
        _secFree(ipc_cryptCommunicate(REQUEST_TERMHTTP, global_state));
        secFree(global_state);
        global_state = NULL;
      }
      break;
    default:
      syslog(LOG_AUTHPRIV | LOG_EMERG, "HttpServer caught Signal %d", signo);
  }
  exit(signo);
}

void registerSignalHandler(const char* state) {
  global_state = oidc_sprintf(state);
  old_sigint   = signal(SIGINT, gen_http_signal_handler);
}

void unregisterSignalHandler() {
  secFree(global_state);
  global_state = NULL;
  signal(SIGINT, old_sigint);
}

void gen_handleUpdateConfigFile(const char* file) {
  if (file == NULL) {
    printError("No shortname provided\n");
  }
  char* fileContent = NULL;
  int   shortname   = 0;
  if (file[0] == '/' || file[0] == '~') {  // absolut path
    fileContent = readFile(file);
  } else {  // file placed in oidc-dir
    fileContent = readOidcFile(file);
    shortname   = 1;
  }
  if (fileContent == NULL) {
    printError("Could not read file '%s'\n", file);
    exit(EXIT_FAILURE);
  }

  char* password  = NULL;
  char* decrypted = NULL;
  for (int i = 0; i < MAX_PASS_TRIES && decrypted == NULL; i++) {
    password =
        promptPassword("Enter decryption Password for the passed file: ");
    decrypted = decryptFileContent(fileContent, password);
    if (decrypted == NULL) {
      oidc_perror();
      secFree(password);
    }
  }
  secFree(fileContent);
  if (decrypted == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (encryptAndWriteWithPassword(decrypted, password, shortname ? NULL : file,
                                  shortname ? file : NULL) != OIDC_SUCCESS) {
    secFree(password);
    secFree(decrypted);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  secFree(password);
  secFree(decrypted);
  printNormal("Updated config file format\n");
  exit(EXIT_SUCCESS);
}

void gen_assertAgent() {
  char* res = ipc_cryptCommunicate(REQUEST_CHECK);
  if (res == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  secFree(res);
}

oidc_error_t gen_handlePublicClient(struct oidc_account* account,
                                    struct arguments*    arguments) {
  arguments->usePublicClient       = 1;
  oidc_gen_state.doNotMergeTmpFile = 1;
  char* old_client_id              = account_getClientId(account);
  updateAccountWithPublicClientInfo(account);
  if (account_getClientId(account) == old_client_id) {
    return OIDC_ENOPUBCLIENT;
  }
  handleGen(account, arguments, NULL);
  return OIDC_SUCCESS;
}
