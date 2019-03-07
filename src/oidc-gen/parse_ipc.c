#define _XOPEN_SOURCE 500
#include "parse_ipc.h"
#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "oidc-gen/gen_handler.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"
#include "utils/uriUtils.h"

#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <unistd.h>

/**
 * @param res a pointer to the response that should be parsed. The pointer will
 * be freed!
 */
char* gen_parseResponse(char* res, const struct arguments* arguments) {
  INIT_KEY_VALUE(IPC_KEY_STATUS, IPC_KEY_CONFIG, OIDC_KEY_ERROR, IPC_KEY_URI,
                 IPC_KEY_INFO, OIDC_KEY_STATE, IPC_KEY_DEVICE);
  if (CALL_GETJSONVALUES(res) < 0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    secFree(res);
    exit(EXIT_FAILURE);
  }
  KEY_VALUE_VARS(status, config, error, uri, info, state, device);
  secFree(res);
  if (_error != NULL) {
    printError("Error: %s\n", _error);
    if (_info) {
      printNormal("%s\n", _info);
    }
    SEC_FREE_KEY_VALUES();
    exit(EXIT_FAILURE);
  }
  if (_config == NULL) {  // res does not contain config
    if (strcaseequal(_status, STATUS_NOTFOUND)) {
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "%s", _info);
      SEC_FREE_KEY_VALUES();
      oidc_errno = OIDC_EWRONGSTATE;
      return NULL;
    }
    if (strcaseequal(_status, STATUS_FOUNDBUTDONE)) {
      printNormal("\n%s\n", _info);
      syslog(LOG_AUTHPRIV | LOG_DEBUG, "%s", _info);
      SEC_FREE_KEY_VALUES();
      return oidc_strcopy(STATUS_FOUNDBUTDONE);
    }
    if (_uri == NULL) {
      printError("Error: response does not contain updated config\n");
    }
  }
  printNormal("%s\n", _status);
  if (strcaseequal(_status, STATUS_SUCCESS)) {
    printNormal(
        "The generated account config was successfully added to oidc-agent. "
        "You don't have to run oidc-add.\n");
  } else if (strcaseequal(_status, STATUS_ACCEPTED)) {
    if (_info) {
      printImportant("%s\n", _info);
    }
    if (_device) {
      char* ret = gen_handleDeviceFlow(_device, _config, arguments);
      SEC_FREE_KEY_VALUES();
      return ret;
    }
    if (_uri) {
      printImportant("To continue and approve the registered client visit the "
                     "following URL in a Browser of your choice:\n%s\n",
                     _uri);
      char* redirect_uri =
          extractParameterValueFromUri(_uri, OIDC_KEY_REDIRECTURI);
      int no_statelookup = 0;
      if (strstarts(redirect_uri, AGENT_CUSTOM_SCHEME)) {
        printImportant("\nYou are using a redirect uri with a custom scheme. "
                       "Your browser will redirect you to a another oidc-gen "
                       "instance automatically. You then can complete the "
                       "account configuration generation process there.\n");
        no_statelookup = 1;
      } else if (arguments->noWebserver) {
        printImportant(
            "\nYou have chosen to not use a webserver. You therefore have to "
            "do a manual redirect. Your browser will redirect you to '%s' "
            "which will not succeed, because oidc-agent did not start a "
            "webserver. Copy the whole url you are being redirected to and "
            "pass it to:\noidc-gen --codeExchange='<url>'\n");
        no_statelookup = 1;
      }
      secFree(redirect_uri);
      char* cmd = oidc_sprintf("xdg-open \"%s\"", _uri);
      system(cmd);
      secFree(cmd);
      if (no_statelookup) {
        exit(EXIT_SUCCESS);
      }
    }
    if (_state) {
      sleep(2);
      secFree(_config);  // should be NULL, but anyway
      _config = configFromStateLookUp(_state, arguments);
    }
  }
  secFree(_status);
  secFreeKeyValuePairs(&pairs[2], sizeof(pairs) / sizeof(*pairs) - 2);
  return _config;
}
