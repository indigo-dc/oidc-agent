#include "gen_handler.h"
#include "account/issuer_helper.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "ipc/cryptCommunicator.h"
#include "list/list.h"
#include "oidc-agent/httpserver/termHttpserver.h"
#include "oidc-agent/oidc/device_code.h"
#include "oidc-gen/gen_signal_handler.h"
#include "oidc-gen/parse_ipc.h"
#include "oidc-gen/promptAndSet.h"
#include "utils/accountUtils.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/password_entry.h"
#include "utils/portUtils.h"
#include "utils/printer.h"
#include "utils/prompt.h"
#include "utils/promptUtils.h"
#include "utils/stringUtils.h"
#include "utils/uriUtils.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
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
  if (strSubStringCase(flow, FLOW_VALUE_PASSWORD) &&
      (!strValid(account_getUsername(account)) ||
       !strValid(account_getPassword(account)))) {
    promptAndSetUsername(account, arguments->flows);
    promptAndSetPassword(account, arguments->flows);
  }
  char* json = accountToJSONString(account);
  printNormal("Generating account configuration ...\n");
  struct password_entry pw   = {.shortname = account_getName(account)};
  unsigned char         type = PW_TYPE_PRMT;
  if (arguments->pw_cmd) {
    pwe_setCommand(&pw, arguments->pw_cmd);
    type |= PW_TYPE_CMD;
  }
  pwe_setType(&pw, type);
  char* pw_str = passwordEntryToJSONString(&pw);
  char* res    = ipc_cryptCommunicate(REQUEST_GEN, json, flow, pw_str,
                                   arguments->noWebserver);
  secFree(flow);
  secFree(json);
  secFree(pw_str);
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

  char* issuer = getJSONValueFromString(json, AGENT_KEY_ISSUERURL);
  updateIssuerConfig(issuer);
  secFree(issuer);

  char* name = getJSONValueFromString(json, AGENT_KEY_SHORTNAME);
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

char* _adjustUriSlash(const char* uri, unsigned char uri_needs_slash) {
  if (uri == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (uri_needs_slash) {
    return uri[strlen(uri) - 1] == '/' ? oidc_strcopy(uri)
                                       : oidc_strcat(uri, "/");
  }
  return uri[strlen(uri) - 1] != '/' ? oidc_strcopy(uri)
                                     : oidc_strncopy(uri, strlen(uri) - 1);
}

void handleCodeExchange(const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  struct codeState codeState   = codeStateFromURI(arguments->codeExchange);
  char*            tmp         = oidc_strcopy(codeState.state);
  char*            uri_slash_s = strtok(tmp, ":");
  /* char*  random_state = */ strtok(NULL, ":");
  char*  len_s              = strtok(NULL, ":");
  char*  socket_path_base64 = strtok(NULL, ":");
  size_t len                = strToULong(len_s);
  char*  socket_path        = NULL;
  if (socket_path_base64 == NULL) {
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "No socket_path encoded in state");
    socket_path = oidc_strcopy(getenv(OIDC_SOCK_ENV_NAME));
    if (socket_path == NULL) {
      printError("Socket path not encoded in url state and not available from "
                 "environment. Cannot connect to oidc-agent.\n");
      exit(EXIT_FAILURE);
    }
  } else {
    socket_path = secAlloc(len + 1);
    fromBase64UrlSafe(socket_path_base64, len, (unsigned char*)socket_path);
  }
  unsigned char uri_needs_slash = strToUChar(uri_slash_s);
  secFree(tmp);

  char* baseUri = _adjustUriSlash(codeState.uri, uri_needs_slash);
  char* uri     = oidc_sprintf("%s?code=%s&state=%s", baseUri, codeState.code,
                           codeState.state);
  secFree(baseUri);
  secFreeCodeState(codeState);
  char* res =
      ipc_cryptCommunicateWithPath(socket_path, REQUEST_CODEEXCHANGEGEN, uri);
  secFree(socket_path);
  secFree(uri);
  if (NULL == res) {
    printError("Error: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  char* config     = gen_parseResponse(res, arguments);
  char* short_name = oidc_strcopy(arguments->args[0]);
  if (!strValid(short_name)) {
    secFree(short_name);
    short_name = getJSONValueFromString(config, AGENT_KEY_SHORTNAME);
  }
  while (!strValid(short_name)) {
    secFree(short_name);
    short_name = prompt("Enter short name for the account to configure: ");
    if (oidcFileDoesExist(short_name)) {
      if (!promptConsentDefaultNo(
              "An account with that shortname already exists. Overwrite?")) {
        secFree(short_name);
      }
    }
  }
  char* hint = oidc_sprintf("account configuration '%s'", short_name);
  encryptAndWriteConfig(config, short_name, hint, NULL, NULL, short_name,
                        arguments->verbose);
  secFree(hint);
  secFree(short_name);
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
  for (; config == NULL && i < MAX_POLL; i++) {
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
    if (config == NULL) {  // TODO check if error really is not found
      sleep(DELTA_POLL);
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
  if (strequal(config, STATUS_FOUNDBUTDONE)) {
    exit(EXIT_SUCCESS);
  }
  unregisterSignalHandler();
  char* issuer = getJSONValueFromString(config, "issuer_url");
  updateIssuerConfig(issuer);
  secFree(issuer);

  char* short_name = getJSONValueFromString(config, AGENT_KEY_SHORTNAME);
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
    INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, IPC_KEY_CONFIG);
    if (CALL_GETJSONVALUES(res) < 0) {
      printError("Could not decode json: %s\n", res);
      printError("This seems to be a bug. Please hand in a bug report.\n");
      SEC_FREE_KEY_VALUES();
      secFree(res);
      exit(EXIT_FAILURE);
    }
    secFree(res);
    KEY_VALUE_VARS(status, error, config);
    if (_error) {
      if (strequal(_error, OIDC_SLOW_DOWN)) {
        interval++;
        SEC_FREE_KEY_VALUES();
        continue;
      }
      if (strequal(_error, OIDC_AUTHORIZATION_PENDING)) {
        SEC_FREE_KEY_VALUES();
        continue;
      }
      printError(_error);
      SEC_FREE_KEY_VALUES();
      exit(EXIT_FAILURE);
    }
    secFree(_status);
    return _config;
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
  promptAndSetName(account, arguments->args[0], arguments->cnid);
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
      account, arguments->flows && strequal(list_at(arguments->flows, 0)->val,
                                            FLOW_VALUE_DEVICE));
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
  promptAndSetName(account, arguments->args[0], arguments->cnid);
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
        account,
        arguments->redirect_uris);  // Note that this will eventually
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
  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, IPC_KEY_CLIENT, IPC_KEY_INFO);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  secFree(res);
  KEY_VALUE_VARS(status, error, client, info);
  if (_error) {
    if (strValid(_client)) {  // if a client was registered, but there's
                              // still an
      // error (i.e. not all required scopes could be
      // registered) temporarily save the client config
      cJSON* json_config = stringToJson(_client);
      jsonAddStringValue(json_config, AGENT_KEY_ISSUERURL,
                         account_getIssuerUrl(account));
      jsonAddStringValue(json_config, AGENT_KEY_CERTPATH,
                         account_getCertPath(account));
      secFree(_client);
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
          char* client_id = getJSONValueFromString(config, OIDC_KEY_CLIENTID);
          char* path      = generateClientConfigFileName(
              account_getIssuerUrl(account), client_id);
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
    if (errorMessageIsForError(_error, OIDC_ENOSUPREG)) {
      printNormal("Dynamic client registration not supported by this "
                  "issuer.\nTry using a public client ...\n");
    } else {
      printNormal("The following error occured during dynamic client "
                  "registration:\n%s\n",
                  _error);
      if (_info) {
        printNormal("%s\n", _info);
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
    exit(EXIT_FAILURE);
  }
  if (_info) {
    printImportant("%s\n", _info);
    secFree(_info);
  }
  secFree(_status);
  secFree(_error);
  if (_client) {
    cJSON* client_config_json = stringToJson(_client);
    secFree(_client);
    cJSON* account_config_json = accountToJSONWithoutCredentials(account);
    cJSON* merged_json =
        mergeJSONObjects(client_config_json, account_config_json);
    secFreeJson(account_config_json);
    secFreeJson(client_config_json);
    char* new_scope_value = getJSONValue(merged_json, OIDC_KEY_SCOPE);
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
        char* client_id = getJSONValueFromString(text, OIDC_KEY_CLIENTID);
        char* path = generateClientConfigFileName(account_getIssuerUrl(account),
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
    secFreeAccount(account);
    return updatedAccount;
  }
  secFreeAccount(account);
  return NULL;
}

void deleteAccount(char* short_name, char* account_json, int revoke) {
  char* res = ipc_cryptCommunicate(revoke ? REQUEST_DELETE : REQUEST_REMOVE,
                                   revoke ? account_json : short_name);

  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  secFree(res);
  KEY_VALUE_VARS(status, error);
  if (strequal(_status, STATUS_SUCCESS) ||
      strequal(_error, ACCOUNT_NOT_LOADED)) {
    printf("The generated account was successfully removed from oidc-agent. "
           "You don't have to run oidc-add.\n");
    SEC_FREE_KEY_VALUES();
    if (removeOidcFile(short_name) == 0) {
      printf("Successfully deleted account configuration.\n");
    } else {
      printError("error removing configuration file: %s", oidc_serror());
    }
    exit(EXIT_SUCCESS);
  }
  if (_error != NULL) {
    printError("Error: %s\n", _error);
    if (strstarts(_error, "Could not revoke token:")) {
      if (promptConsentDefaultNo(
              "Do you want to unload and delete anyway. You then have to "
              "revoke the refresh token manually.")) {
        deleteAccount(short_name, account_json, 0);
      } else {
        printError(
            "The account was not removed from oidc-agent due to the above "
            "listed error. You can fix the error and try it again.\n");
      }
    } else {
      printError("The account was not removed from oidc-agent due to the above "
                 "listed error. You can fix the error and try it again.\n");
    }
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
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
  deleteAccount(arguments->args[0], json, 1);
  secFree(json);
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
      printNormal("The following data will be saved encrypted:\n%s\n", config);
    }
    return promptEncryptAndWriteText(config, hint, suggestedPassword, filepath,
                                     oidc_filename);
  }
  char* tmpcontent = readFile(tmpFile);
  char* text       = mergeJSONObjectStrings(config, tmpcontent);
  secFree(tmpcontent);
  oidc_error_t merge_error = OIDC_SUCCESS;
  if (text == NULL) {
    oidc_perror();
    merge_error = oidc_errno;
    printError("Only saving the account configuration. You might want to "
               "save the content of '%s' in another location.\n",
               tmpFile);
    secFree(tmpFile);
    text = oidc_strcopy(config);
  }
  if (verbose) {
    printNormal("The following data will be saved encrypted:\n%s\n", text);
  }
  oidc_error_t e = promptEncryptAndWriteText(text, hint, suggestedPassword,
                                             filepath, oidc_filename);
  secFree(text);
  if (e == OIDC_SUCCESS && merge_error == OIDC_SUCCESS) {
    removeFile(tmpFile);
  }
  secFree(tmpFile);
  return e;
}

void gen_handleList() {
  list_t* list = getClientConfigFileList();
  char*   str  = listToDelimitedString(list, '\n');
  list_destroy(list);
  printf("The following client configuration files are usable:\n%s\n", str);
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

  char* password = NULL;
  if (isJSONObject(fileContent)) {  // file not encrypted
    password = getEncryptionPassword(file, NULL, UINT_MAX);
  } else {  // file is encrypted
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
    fileContent = decrypted;
  }
  if (encryptAndWriteUsingPassword(fileContent, password,
                                   shortname ? NULL : file,
                                   shortname ? file : NULL) != OIDC_SUCCESS) {
    secFree(password);
    secFree(fileContent);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  secFree(password);
  secFree(fileContent);
  printNormal("Updated config file format\n");
  exit(EXIT_SUCCESS);
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
