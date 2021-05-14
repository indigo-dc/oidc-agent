#include "gen_handler.h"
#include "account/account.h"
#include "account/issuer_helper.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "ipc/cryptCommunicator.h"
#include "oidc-agent/httpserver/termHttpserver.h"
#include "oidc-agent/oidc/device_code.h"
#include "oidc-gen/gen_consenter.h"
#include "oidc-gen/gen_signal_handler.h"
#include "oidc-gen/parse_ipc.h"
#include "oidc-gen/promptAndSet/promptAndSet.h"
#include "oidc-token/parse.h"
#include "utils/accountUtils.h"
#include "utils/crypt/crypt.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/errorUtils.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/parseJson.h"
#include "utils/password_entry.h"
#include "utils/portUtils.h"
#include "utils/printer.h"
#include "utils/prompt.h"
#include "utils/promptUtils.h"
#include "utils/pubClientInfos.h"
#include "utils/stringUtils.h"
#include "utils/uriUtils.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define IGNORE_ERROR 1

struct state {
  int doNotMergeTmpFile;
} oidc_gen_state;

static const unsigned char remote = 0;

char* _gen_response(struct oidc_account*    account,
                    const struct arguments* arguments) {
  readDeviceAuthEndpoint(account, arguments);
  readAudience(account, arguments);
  cJSON* flow_json = listToJSONArray(arguments->flows);
  char*  log_tmp   = jsonToString(flow_json);
  logger(DEBUG, "arguments flows in handleGen are '%s'", log_tmp);
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
  logger(DEBUG, "flows in handleGen are '%s'", flow);
  if (strSubStringCase(flow, FLOW_VALUE_PASSWORD) &&
      (!strValid(account_getUsername(account)) ||
       !strValid(account_getPassword(account)))) {
    needUsername(account, arguments);
    needPassword(account, arguments);
  }
  char* json = accountToJSONString(account);
  if (!arguments->only_at) {
    printStdout("Generating account configuration ...\n");
  }
  struct password_entry pw   = {.shortname = account_getName(account)};
  unsigned char         type = PW_TYPE_PRMT;
  if (arguments->pw_cmd) {
    pwe_setCommand(&pw, arguments->pw_cmd);
    type |= PW_TYPE_CMD;
  }
  if (arguments->pw_file) {
    pwe_setFile(&pw, arguments->pw_file);
    type |= PW_TYPE_FILE;
  }
  pwe_setType(&pw, type);
  char* pw_str = passwordEntryToJSONString(&pw);
  char* res    = ipc_cryptCommunicate(remote, REQUEST_GEN, json, flow, pw_str,
                                   arguments->noWebserver, arguments->noScheme,
                                   arguments->only_at);
  secFree(flow);
  secFree(json);
  secFree(pw_str);
  if (NULL == res) {
    printError("Error: %s\n", oidc_serror());
    secFreeAccount(account);
    exit(EXIT_FAILURE);
  }
  return gen_parseResponse(res, arguments);
}

void handleGen(struct oidc_account* account, const struct arguments* arguments,
               const char* suggested_password) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }

  char* json   = _gen_response(account, arguments);
  char* issuer = getJSONValueFromString(json, AGENT_KEY_ISSUERURL);
  char* name   = getJSONValueFromString(json, AGENT_KEY_SHORTNAME);
  if (!arguments->noSave) {
    updateIssuerConfig(issuer, name);
  }
  secFree(issuer);
  char* hint = oidc_sprintf("account configuration '%s'", name);
  gen_saveAccountConfig(json, account_getName(account), hint,
                        suggested_password, arguments);
  secFree(hint);
  secFree(name);
  secFreeAccount(account);
  secFree(json);
}

void manualGen(struct oidc_account*    account,
               const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  char*  cryptPass    = NULL;
  char** cryptPassPtr = &cryptPass;
  account             = manual_genNewAccount(account, arguments, cryptPassPtr);
  cryptPass           = *cryptPassPtr;
  handleGen(account, arguments, cryptPass);
  secFree(cryptPass);
}

void reauthenticate(const char* shortname, const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (shortname == NULL) {
    printError(
        "You have to specify a shortname to update the refresh token for it\n");
    exit(EXIT_FAILURE);
  }
  if (!oidcFileDoesExist(shortname)) {
    printError("No account configuration found with that shortname\n");
    exit(EXIT_FAILURE);
  }
  struct resultWithEncryptionPassword result =
      getDecryptedAccountAndPasswordFromFilePrompt(
          shortname, arguments->pw_cmd, arguments->pw_file, arguments->pw_env);
  if (result.result == NULL) {
    oidc_perror();
    secFree(result.password);
    exit(EXIT_FAILURE);
  }
  handleGen(result.result, arguments, result.password);
  exit(EXIT_SUCCESS);
}

void gen_handleRename(const char*             shortname,
                      const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (shortname == NULL) {
    printError("You have to specify a shortname to rename it\n");
    exit(EXIT_FAILURE);
  }
  if (!oidcFileDoesExist(shortname)) {
    printError("No account configuration found with that shortname\n");
    exit(EXIT_FAILURE);
  }
  const char* new_shortname = arguments->rename;
  if (new_shortname == NULL) {
    printError("You have to specify the new shortname to rename '%s'\n",
               shortname);
    exit(EXIT_FAILURE);
  }
  if (oidcFileDoesExist(new_shortname)) {
    printError("Account configuration '%s' already exists\n", new_shortname);
    exit(EXIT_FAILURE);
  }
  struct resultWithEncryptionPassword result =
      getDecryptedAccountAndPasswordFromFilePrompt(
          shortname, arguments->pw_cmd, arguments->pw_file, arguments->pw_env);
  if (result.result == NULL) {
    oidc_perror();
    secFree(result.password);
    exit(EXIT_FAILURE);
  }
  struct oidc_account* account = result.result;
  secFree(account->shortname);
  account->shortname =
      oidc_strcopy(new_shortname);  // We don't use account_setName since this
                                    // also updates the client name

  char* hint = oidc_sprintf("account configuration '%s'", new_shortname);
  char* json = accountToJSONString(account);
  if (gen_saveAccountConfig(json, account_getName(account), hint,
                            result.password, arguments) != OIDC_SUCCESS) {
    oidc_perror();
  } else {
    if (removeOidcFile(shortname) != 0) {
      printError("error removing old configuration file: %s", oidc_serror());
    }
  }
  secFree(json);
  secFree(hint);
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
  char* error = extractParameterValueFromUri(arguments->codeExchange, "error");
  if (error) {
    char* error_description = extractParameterValueFromUri(
        arguments->codeExchange, "error_description");
    char* err = combineError(error, error_description);
    logger(ERROR, "HttpRedirect Error: %s", err);
    secFree(error_description);
    secFree(error);
    oidc_seterror(err);
    oidc_errno = OIDC_EOIDC;
    secFree(err);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  struct codeState codeState = codeStateFromURI(arguments->codeExchange);
  if (codeState.state == NULL) {
    printError("Uri does not contain a state\n");
    exit(EXIT_FAILURE);
  }
  if (codeState.code == NULL) {
    printError("Uri does not contain a code\n");
    exit(EXIT_FAILURE);
  }
  char* tmp         = oidc_strcopy(codeState.state);
  char* uri_slash_s = strtok(tmp, ":");
  /* char*  random_state = */ strtok(NULL, ":");
  char*  len_s              = strtok(NULL, ":");
  char*  socket_path_base64 = strtok(NULL, ":");
  size_t len                = strToULong(len_s);
  char*  socket_path        = NULL;
  if (socket_path_base64 == NULL) {
    logger(NOTICE, "No socket_path encoded in state");
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
    short_name =
        prompt("Enter short name for the account to configure: ", "short name",
               NULL, CLI_PROMPT_VERBOSE);
    if (oidcFileDoesExist(short_name)) {
      if (!gen_promptConsentDefaultNo(
              "An account with that shortname already exists. Overwrite?",
              arguments)) {
        secFree(short_name);
      }
    }
  }
  char* hint = oidc_sprintf("account configuration '%s'", short_name);
  gen_saveAccountConfig(config, short_name, hint, NULL, arguments);
  secFree(hint);
  secFree(short_name);
  secFree(config);
}

char* singleStateLookUp(const char* state, const struct arguments* arguments) {
  char* res = ipc_cryptCommunicate(remote, REQUEST_STATELOOKUP, state);
  if (NULL == res) {
    printStdout("\n");
    printError("Error: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  if (arguments->verbose) {
    printStdout("%s\n", res);
  }
  char* config = gen_parseResponse(res, arguments);
  return config;
}

char* configFromStateLookUp(const char*             state,
                            const struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  registerSignalHandler(state);
  char* config = NULL;
  printNormal(
      "Polling oidc-agent to get the generated account configuration ...");
  fflush(stderr);
  for (unsigned int i = 0; config == NULL && i < MAX_POLL; i++) {
    config = singleStateLookUp(state, arguments);
    if (config == NULL) {
      sleep(DELTA_POLL);
      printNormal(".");
      fflush(stderr);
    }
  }
  printNormal("\n");
  if (config == NULL) {
    printNormal("Polling is boring. Already tried %d times. I stop now.\n",
                MAX_POLL);
    printImportant("Please press Enter to try it again.\n");
    getchar();
    config = singleStateLookUp(state, arguments);
    if (config == NULL) {
      printError("Could not receive generated account configuration for "
                 "state='%s'\n",
                 state);
      printImportant(
          "Please try state lookup again by using:\noidc-gen --state=%s\n",
          state);
      _secFree(ipc_cryptCommunicate(remote, REQUEST_TERMHTTP, state));
      exit(EXIT_FAILURE);
    }
  }
  unregisterSignalHandler();
  if (strequal(config, STATUS_FOUNDBUTDONE)) {
    exit(EXIT_SUCCESS);
  }
  return config;
}

void stateLookUpWithConfigSave(const char*             state,
                               const struct arguments* arguments) {
  char* config = singleStateLookUp(state, arguments);
  if (config == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  char* issuer     = getJSONValueFromString(config, AGENT_KEY_ISSUERURL);
  char* short_name = getJSONValueFromString(config, AGENT_KEY_SHORTNAME);
  if (!arguments->noSave) {
    updateIssuerConfig(issuer, short_name);
  }
  secFree(issuer);
  char* hint = oidc_sprintf("account configuration '%s'", short_name);
  gen_saveAccountConfig(config, short_name, hint, NULL, arguments);
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
  printDeviceCode(*dc);
  size_t interval   = oidc_device_getInterval(*dc);
  size_t expires_in = oidc_device_getExpiresIn(*dc);
  long   expires_at = time(NULL) + expires_in;
  secFreeDeviceCode(dc);
  while (expires_in ? expires_at > time(NULL) : 1) {
    sleep(interval);
    char* res = ipc_cryptCommunicate(remote, REQUEST_DEVICE, json_device,
                                     json_account, arguments->only_at);
    INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, IPC_KEY_CONFIG,
                   OIDC_KEY_ACCESSTOKEN);
    if (CALL_GETJSONVALUES(res) < 0) {
      printError("Could not decode json: %s\n", res);
      printError("This seems to be a bug. Please hand in a bug report.\n");
      SEC_FREE_KEY_VALUES();
      secFree(res);
      exit(EXIT_FAILURE);
    }
    secFree(res);
    KEY_VALUE_VARS(status, error, config, at);
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
    if (arguments->only_at) {
      secFree(_config);
    } else {
      secFree(_at);
    }
    return arguments->only_at ? _at : _config;
  }
  printError("Device code is not valid any more!");
  exit(EXIT_FAILURE);
}

struct oidc_account* manual_genNewAccount(struct oidc_account*    account,
                                          const struct arguments* arguments,
                                          char** cryptPassPtr) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (account == NULL) {
    account = secAlloc(sizeof(struct oidc_account));
  }
  if (!arguments->only_at) {
    needName(account, arguments);
    char* shortname = account_getName(account);
    if (oidcFileDoesExist(shortname)) {
      struct resultWithEncryptionPassword result =
          getDecryptedAccountAndPasswordFromFilePrompt(
              shortname, arguments->pw_cmd, arguments->pw_file,
              arguments->pw_env);
      if (result.result == NULL) {
        oidc_perror();
        secFree(result.password);
        exit(EXIT_FAILURE);
      }
      secFreeAccount(account);
      account       = result.result;
      *cryptPassPtr = result.password;
    } else {
      printStdout(
          "No account exists with this short name. Creating new configuration "
          "...\n");
      char* tmpFile = oidc_strcat(CLIENT_TMP_PREFIX, shortname);
      char* tmpData = readFileFromAgent(tmpFile, IGNORE_ERROR);
      if (tmpData != NULL) {
        if (gen_promptConsentDefaultYes(
                "Found temporary file for this shortname. Do "
                "you want to use it?",
                arguments)) {
          secFreeAccount(account);
          account = getAccountFromJSON(tmpData);
        } else {
          oidc_gen_state.doNotMergeTmpFile = 1;
        }
      }
      secFree(tmpData);
      secFree(tmpFile);
    }
  }
  readCertPath(account, arguments);
  needIssuer(account, arguments);
  needClientId(account, arguments);
  askOrNeedClientSecret(account, arguments, arguments->usePublicClient);
  needScope(account, arguments);
  readAudience(account, arguments);
  readRefreshToken(account, arguments);
  if (findInList(arguments->flows, FLOW_VALUE_PASSWORD)) {
    needUsername(account, arguments);
    needPassword(account, arguments);
  }
  int redirectUrisOptional =                   // redirectUris are not needed if
      account_refreshTokenIsValid(account) ||  // RT provided OR
      (strValid(account_getUsername(account)) &&
       strValid(
           account_getPassword(account))) ||  // User Credentials provided OR
      (arguments->flows && strequal(list_at(arguments->flows, 0)->val,
                                    FLOW_VALUE_DEVICE)  // Device Flow
      );
  askOrNeedRedirectUris(account, arguments, redirectUrisOptional);
  return account;
}

struct oidc_account* registerClient(struct arguments* arguments) {
  if (arguments == NULL) {
    oidc_setArgNullFuncError(__func__);
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  struct oidc_account* account = secAlloc(sizeof(struct oidc_account));
  needName(account, arguments);
  if (oidcFileDoesExist(account_getName(account))) {
    printError("An account with that shortname is already configured\n");
    exit(EXIT_FAILURE);
  }

  char* tmpFile = oidc_strcat(CLIENT_TMP_PREFIX, account_getName(account));
  char* tmpData = readFileFromAgent(tmpFile, IGNORE_ERROR);
  if (tmpData != NULL && !arguments->usePublicClient) {
    if (gen_promptConsentDefaultYes(
            "Found temporary file for this shortname. Do "
            "you want to use it?",
            arguments)) {
      secFreeAccount(account);
      account = getAccountFromJSON(tmpData);
      handleGen(account, arguments, NULL);
      exit(EXIT_SUCCESS);
    } else {
      oidc_gen_state.doNotMergeTmpFile = 1;
    }
  }
  secFree(tmpFile);
  secFree(tmpData);

  readCertPath(account, arguments);
  needIssuer(account, arguments);
  readDeviceAuthEndpoint(account, arguments);
  needScope(account, arguments);

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

  readRedirectUris(account, arguments);

  char* json = accountToJSONString(account);
  printStdout("Registering Client ...\n");
  char* flows = listToJSONArrayString(arguments->flows);
  char* res   = ipc_cryptCommunicate(remote, REQUEST_REGISTER_AUTH, json, flows,
                                   arguments->dynRegToken ?: "");
  secFree(flows);
  secFree(json);
  if (NULL == res) {
    printError("Error: %s\n", oidc_serror());
    secFreeAccount(account);
    exit(EXIT_FAILURE);
  }
  if (arguments->verbose && res) {
    printStdout("%s\n", res);
  }
  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, IPC_KEY_CLIENT, IPC_KEY_INFO,
                 IPC_KEY_MAXSCOPES);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  secFree(res);
  KEY_VALUE_VARS(status, error, client, info, max_scopes);
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
      char* path = oidc_strcat(CLIENT_TMP_PREFIX, account_getName(account));
      if (arguments->verbose) {
        printStdout("Writing client config temporary to agent\n");
      }
      writeFileToAgent(path, config);
      secFree(path);
    }

    // if dyn reg not possible try using a preregistered public client
    if (errorMessageIsForError(_error, OIDC_ENOSUPREG)) {
      printStdout("Dynamic client registration not supported by this "
                  "issuer.\nTry using a public client ...\n");
    } else if (strstarts(_error,
                         "An account with this shortname is already loaded.")) {
      printError("%s\n", _error);
      exit(EXIT_FAILURE);
    } else {
      printNormal("The following error occurred during dynamic client "
                  "registration:\n%s\n",
                  _error);
      if (_info) {
        printNormal("%s\n", _info);
      }
      printStdout("Try using a public client ...\n");
    }
    oidc_error_t pubError = gen_handlePublicClient(account, arguments);
    switch (pubError) {
      case OIDC_SUCCESS:
        // Actually we should already have exited
        exit(EXIT_SUCCESS);
      case OIDC_ENOPUBCLIENT:
        printError("Dynamic client registration not successful for this issuer "
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
    const char* requested_scope = account_getScope(account);
    if (strequal(requested_scope, "max") && _max_scopes) {
      requested_scope = _max_scopes;
    }
    char* scope_diff =
        subtractListStrings(requested_scope, new_scope_value, ' ');
    secFree(new_scope_value);
    if (scope_diff) {
      printImportant(
          "Warning: The registered client does not have all the requested "
          "scopes. The following are missing: %s\nTo update the client to have "
          "all the requested scope values, please contact the provider.\n",
          scope_diff);
      printIssuerHelp(account_getIssuerUrl(account));
      secFree(scope_diff);
    }
    char* text = jsonToString(merged_json);
    secFreeJson(merged_json);
    if (text == NULL) {
      oidc_perror();
      exit(EXIT_FAILURE);
    }
    char* path = oidc_strcat(CLIENT_TMP_PREFIX, account_getName(account));
    if (arguments->verbose) {
      printStdout("Writing client config temporary to agent\n");
    }
    writeFileToAgent(path, text);
    oidc_gen_state.doNotMergeTmpFile = 0;
    secFree(path);
    struct oidc_account* updatedAccount = getAccountFromJSON(text);
    secFree(text);
    secFreeAccount(account);
    secFree(_max_scopes);
    return updatedAccount;
  } else {
    printError("Something went wrong. I did not receive a client config!\n");
  }
  secFree(_max_scopes);
  secFreeAccount(account);
  return NULL;
}

void deleteAccount(char* short_name, char* file_json, int revoke,
                   const struct arguments* arguments) {
  char* refresh_token = NULL;
  if (file_json) {
    INIT_KEY_VALUE(OIDC_KEY_REGISTRATION_ACCESS_TOKEN,
                   OIDC_KEY_REGISTRATION_CLIENT_URI, AGENT_KEY_CERTPATH,
                   OIDC_KEY_REFRESHTOKEN);
    if (CALL_GETJSONVALUES(file_json) < 0) {
      printError("Could not decode json: %s\n", file_json);
      printError("This seems to be a bug. Please hand in a bug report.\n");
      SEC_FREE_KEY_VALUES();
      exit(EXIT_FAILURE);
    }
    KEY_VALUE_VARS(registration_access_token, registration_client_uri, certpath,
                   refresh_token);
    refresh_token =
        oidc_strcopy(_refresh_token);  // Save this for later, so the RT can be
                                       // given to the user, if not revoked.
    if (_registration_access_token && _registration_client_uri) {
      if (gen_promptConsentDefaultYes(
              "The used OIDC client was dynamically registered by oidc-agent. "
              "Should this client be automatically deleted?",
              arguments)) {
        char* res = ipc_cryptCommunicate(remote, REQUEST_DELETECLIENT,
                                         _registration_client_uri,
                                         _registration_access_token, _certpath);
        {  // Capsulate json parsing macros
          INIT_KEY_VALUE(OIDC_KEY_ERROR);
          if (CALL_GETJSONVALUES(res) < 0) {
            printError("Could not decode json: %s\n", res);
            printError(
                "This seems to be a bug. Please hand in a bug report.\n");
            secFree(res);
            SEC_FREE_KEY_VALUES();
            exit(EXIT_FAILURE);
          }
          secFree(res);
          KEY_VALUE_VARS(error);
          if (_error != NULL) {
            printError("%s\n", _error);
            secFree(_error);
            exit(EXIT_FAILURE);
          }
          revoke = 0;
          printNormal("Deleted oidc client.\n");
        }
      }
    }
    SEC_FREE_KEY_VALUES();  // refers to the registration_access_token, etc.
                            // values
  }

  struct oidc_account* p            = getAccountFromJSON(file_json);
  char*                account_json = accountToJSONStringWithoutCredentials(p);
  secFreeAccount(p);

  char* res =
      ipc_cryptCommunicate(remote, revoke ? REQUEST_DELETE : REQUEST_REMOVE,
                           revoke ? account_json : short_name);
  secFree(account_json);
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
    printStdout(
        "The generated account was successfully removed from oidc-agent. "
        "You don't have to run oidc-add.\n");
    SEC_FREE_KEY_VALUES();
    if (removeOidcFile(short_name) == 0) {
      printStdout("Successfully deleted account configuration.\n");
    } else {
      printError("error removing configuration file: %s", oidc_serror());
    }
    exit(EXIT_SUCCESS);
  }
  if (_error != NULL) {
    printError("Error: %s\n", _error);
    if (strstarts(_error, "Could not revoke token:")) {
      if (gen_promptConsentDefaultNo(
              "Do you want to unload and delete anyway. You then have to "
              "revoke the refresh token manually.",
              arguments)) {
        if (refresh_token) {
          printImportant("Please revoke the refresh token '%s' manually!\n",
                         refresh_token);
          secFree(refresh_token);
        }
        deleteAccount(short_name, file_json, 0, arguments);
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
  char* json = getDecryptedOidcFileFor(arguments->args[0], arguments->pw_cmd,
                                       arguments->pw_file, arguments->pw_env);
  if (json == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  if (gen_promptConsentDefaultNo(
          "Do you really want to delete this configuration?", arguments)) {
    deleteAccount(arguments->args[0], json, 1, arguments);
  }
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
oidc_error_t gen_saveAccountConfig(const char* config, const char* shortname,
                                   const char*             hint,
                                   const char*             suggestedPassword,
                                   const struct arguments* arguments) {
  // just skip, if must not save our configs
  if (arguments->noSave) {
    return OIDC_SUCCESS;
  }
  char* tmpFile = oidc_strcat(CLIENT_TMP_PREFIX, shortname);
  char* tmpData = readFileFromAgent(tmpFile, IGNORE_ERROR);
  if (oidc_gen_state.doNotMergeTmpFile || tmpData == NULL) {
    secFree(tmpFile);
    if (arguments->verbose) {
      printStdout("The following data will be saved encrypted:\n%s\n", config);
    }
    return promptEncryptAndWriteToOidcFile(
        config, shortname, hint, suggestedPassword, arguments->pw_cmd,
        arguments->pw_file, arguments->pw_env);
  }
  char*        text        = mergeJSONObjectStrings(config, tmpData);
  oidc_error_t merge_error = OIDC_SUCCESS;
  if (text == NULL) {
    oidc_perror();
    merge_error = oidc_errno;
    printError("Only saving the account configuration. You might want to "
               "save the following content to another location.\n\n%s\n\n",
               tmpData);
    text = oidc_strcopy(config);
  }
  secFree(tmpData);
  if (arguments->verbose) {
    printStdout("The following data will be saved encrypted:\n%s\n", text);
  }
  oidc_error_t e = promptEncryptAndWriteToOidcFile(
      text, shortname, hint, suggestedPassword, arguments->pw_cmd,
      arguments->pw_file, arguments->pw_env);
  secFree(text);
  if (e == OIDC_SUCCESS && merge_error == OIDC_SUCCESS) {
    removeFileFromAgent(tmpFile);
  }
  secFree(tmpFile);
  return e;
}

void gen_handlePrint(const char* file, const struct arguments* arguments) {
  if (file == NULL || strlen(file) < 1) {
    printError("FILE not specified\n");
  }
  char* fileContent = NULL;
  if (file[0] == '/' || file[0] == '~') {  // absolut path
    fileContent = getDecryptedFileFor(file, arguments->pw_cmd,
                                      arguments->pw_file, arguments->pw_env);
  } else {  // file placed in oidc-dir
    fileContent = getDecryptedOidcFileFor(
        file, arguments->pw_cmd, arguments->pw_file, arguments->pw_env);
  }
  if (fileContent == NULL) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
  printStdout("%s\n", fileContent);
  secFree(fileContent);
}
void gen_handleUpdateConfigFile(const char*             file,
                                const struct arguments* arguments) {
  if (file == NULL) {
    printError("No shortname provided\n");
  }
  int isShortname = 0;
  if (file[0] != '/' && file[0] != '~') {  // no absolut path
    isShortname = 1;
  }
  char* (*readFnc)(const char*) = isShortname ? readOidcFile : readFile;
  char* fileContent             = readFnc(file);
  if (isJSONObject(fileContent)) {
    oidc_error_t (*writeFnc)(const char*, const char*, const char*, const char*,
                             const char*, const char*, const char*) =
        isShortname ? promptEncryptAndWriteToOidcFile
                    : promptEncryptAndWriteToFile;
    oidc_error_t write_e =
        writeFnc(fileContent, file, file, NULL, arguments->pw_cmd,
                 arguments->pw_file, arguments->pw_env);
    secFree(fileContent);
    if (write_e != OIDC_SUCCESS) {
      oidc_perror();
    } else {
      printStdout("Updated config file format\n");
    }
    exit(write_e);
  }
  struct resultWithEncryptionPassword result =
      _getDecryptedTextAndPasswordWithPromptFor(
          fileContent, file, decryptFileContent, isShortname, arguments->pw_cmd,
          arguments->pw_file, arguments->pw_env);
  secFree(fileContent);
  if (result.result == NULL) {
    secFree(result.password);
    oidc_perror();
    exit(EXIT_FAILURE);
  }

  oidc_error_t (*writeFnc)(const char*, const char*, const char*) =
      isShortname ? encryptAndWriteToOidcFile : encryptAndWriteToFile;
  oidc_error_t write_e = writeFnc(result.result, file, result.password);
  secFree(result.password);
  secFree(result.result);
  if (write_e != OIDC_SUCCESS) {
    oidc_perror();
  } else {
    printStdout("Updated config file format\n");
  }
  exit(write_e);
}

oidc_error_t gen_handlePublicClient(struct oidc_account* account,
                                    struct arguments*    arguments) {
  arguments->usePublicClient       = 1;
  oidc_gen_state.doNotMergeTmpFile = 1;
  char* old_client_id              = account_getClientId(account);
  if (updateAccountWithPublicClientInfo(account) == NULL) {
    return oidc_errno;
  }
  if (account_getClientId(account) == old_client_id) {
    return OIDC_ENOPUBCLIENT;
  }
  handleGen(account, arguments, NULL);
  return OIDC_SUCCESS;
}

char* gen_handleScopeLookup(const char* issuer_url, const char* cert_path) {
  char* res =
      ipc_cryptCommunicate(remote, REQUEST_SCOPES, issuer_url, cert_path);

  INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, IPC_KEY_INFO);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  secFree(res);
  KEY_VALUE_VARS(status, error, scopes);
  if (!strequal(_status, STATUS_SUCCESS)) {
    printError("Error while retrieving supported scopes for '%s': %s\n",
               issuer_url, _error);
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  secFree(_status);
  return _scopes;
}

char* readFileFromAgent(const char* filename, int ignoreError) {
  char* res = ipc_cryptCommunicate(remote, REQUEST_FILEREAD, filename);
  INIT_KEY_VALUE(OIDC_KEY_ERROR, OIDC_KEY_ERROR_DESCRIPTION, IPC_KEY_DATA);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  secFree(res);
  KEY_VALUE_VARS(error, error_description, data);
  if (_error_description) {
    char* error = combineError(_error, _error_description);
    SEC_FREE_KEY_VALUES();
    if (!ignoreError) {
      printError("%s\n", error);
    }
    secFree(error);
    return NULL;
  }
  if (_error) {
    if (!ignoreError) {
      printError("%s\n", _error);
    }
    secFree(_error);
    return NULL;
  }
  if (_data == NULL) {
    return NULL;
  }
  char* data = secAlloc(strlen(_data));
  fromBase64UrlSafe(_data, strlen(_data), (unsigned char*)data);
  secFree(_data);
  logger(DEBUG, "Decoded base64 file content is: '%s'", data);
  return data;
}

void writeFileToAgent(const char* filename, const char* data) {
  char* data64 = toBase64UrlSafe(data, strlen(data) + 1);
  char* res = ipc_cryptCommunicate(remote, REQUEST_FILEWRITE, filename, data64);
  secFree(data64);
  char* error = parseForError(res);
  if (error != NULL) {
    printError("%s\n", error);
  }
}

void removeFileFromAgent(const char* filename) {
  char* res   = ipc_cryptCommunicate(remote, REQUEST_FILEREMOVE, filename);
  char* error = parseForError(res);
  if (error != NULL) {
    printError("%s\n", error);
  }
}

void handleOnlyAT(struct arguments* arguments) {
  struct oidc_account* account = NULL;
  if (!arguments->manual) {  // On default try using a public client.
    account = secAlloc(sizeof(struct oidc_account));
    needIssuer(account, arguments);
    updateAccountWithPublicClientInfo(account);
    arguments->usePublicClient = 1;
  }
  if (arguments->file) {
    account = getAccountFromMaybeEncryptedFile(arguments->file);
  }
  account = manual_genNewAccount(account, arguments, NULL);

  char* at = _gen_response(account, arguments);
  printStdout("%s\n", at);
}
