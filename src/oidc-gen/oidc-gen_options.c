#include "oidc-gen_options.h"

#include <stdlib.h>

#include "defines/agent_values.h"
#include "defines/settings.h"
#include "utils/commonFeatures.h"
#include "utils/guiChecker.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/portUtils.h"
#include "utils/printer.h"
#include "utils/prompt_mode.h"
#include "utils/string/stringUtils.h"
#include "utils/uriUtils.h"

#ifdef __MSYS__
#include "utils/registryConnector.h"
#endif
/* Keys for options without short-options. */
#define OPT_codeExchange 1
#define OPT_state 2
#define OPT_TOKEN 3
#define OPT_CERTPATH 4
#define OPT_DEVICE 7
#define OPT_CNID 8
#define OPT_NOURLCALL 10
#define OPT_REFRESHTOKEN 11
#define OPT_PUBLICCLIENT 12
#define OPT_PORT 13
#define OPT_PW_CMD 14
#define OPT_NO_WEBSERVER 15
#define OPT_REAUTHENTICATE 16
#define OPT_NO_SCHEME 17
#define OPT_AUDIENCE 18
#define OPT_RENAME 19
#define OPT_PROMPT_MODE 20
#define OPT_PW_PROMPT_MODE 21
#define OPT_ISSUER 22
#define OPT_SCOPE 23
#define OPT_SCOPE_MAX 24
#define OPT_CLIENTID 25
#define OPT_CLIENTSECRET 26
#define OPT_REDIRECT 27
#define OPT_USERNAME 28
#define OPT_PASSWORD 29
#define OPT_PW_FILE 30
// Leave space for Ascii characters
#define OPT_CONFIRM_YES 128
#define OPT_CONFIRM_NO 129
#define OPT_CONFIRM_DEFAULT 130
#define OPT_ONLY_AT 131
#define OPT_REFRESHTOKEN_ENV 132
#define OPT_PW_ENV 133
#define OPT_NO_SAVE 134
#define OPT_PW_GPG 135
#define OPT_CONFIG_ENDPOINT 136
#define OPT_OAUTH 137

static struct argp_option options[] = {
    {0, 0, 0, 0, "Managing account configurations", 1},
    {"accounts", 'l', 0, 0,
     "Prints a list of all configured account configurations. Same as oidc-add "
     "-l",
     1},
    {"print", 'p', "FILE", 0,
     "Prints the decrypted content of FILE. FILE can be an absolute path or "
     "the name of a file placed in oidc-dir (e.g. an account configuration "
     "short name)",
     1},
    {"reauthenticate", OPT_REAUTHENTICATE, 0, 0,
     "Used to update an existing account configuration file with a new refresh "
     "token. Can be used if no other metadata should be changed.",
     1},
    {"rename", OPT_RENAME, "NEW_SHORTNAME", 0,
     "Used to rename an existing account configuration file.", 1},
    {"update", 'u', "FILE", 0,
     "Decrypts and reencrypts the content for FILE. "
     "This might update the file format and encryption. FILE can be an "
     "absolute path or the name of a file placed in oidc-dir (e.g. an account "
     "configuration short name).",
     1},
    {"delete", 'd', 0, 0, "Delete configuration for the given account", 1},

    {0, 0, 0, 0, "Generating a new account configuration:", 2},
    {"file", 'f', "FILE", 0,
     "Reads the client configuration from FILE. Implicitly sets -m", 2},
    {"manual", 'm', 0, 0,
     "Does not use Dynamic Client Registration. Client has to be manually "
     "registered beforehand",
     2},
    {"no-save", OPT_NO_SAVE, 0, 0,
     "Do not save any configuration files (meaning as soon as the agent stops, "
     "nothing will be saved)",
     2},
    {"pub", OPT_PUBLICCLIENT, 0, 0,
     "Uses a public client defined in the publicclient.conf file.", 2},
    {"iss", OPT_ISSUER, "ISSUER_URL", 0,
     "Set ISSUER_URL as the issuer url to be used.", 2},
    {OPT_LONG_ISSUER, OPT_ISSUER, "ISSUER_URL", OPTION_ALIAS, NULL, 2},
    {OPT_LONG_SCOPE, OPT_SCOPE, "SCOPE", 0,
     "Set SCOPE as the scope to be used. Multiple scopes can be provided as a "
     "space separated list or by using the option multiple times. Use 'max' to "
     "use all available scopes for this provider.",
     2},
    {"scope-all", OPT_SCOPE_MAX, 0, 0,
     "Use all available scopes for this provider. Same as using '--scope=max'",
     2},
    {"scope-max", OPT_SCOPE_MAX, 0, OPTION_ALIAS, NULL, 2},
    {OPT_LONG_CLIENTID, OPT_CLIENTID, "CLIENT_ID", 0,
     "Use CLIENT_ID as client id. Requires an already registered client. "
     "Implicitly sets '-m'.",
     2},
    {OPT_LONG_CLIENTSECRET, OPT_CLIENTSECRET, "CLIENT_SECRET", 0,
     "Use CLIENT_SECRET as client secret. Requires an already registered "
     "client.",
     2},
    {OPT_LONG_REDIRECT, OPT_REDIRECT, "URI", 0,
     "Use URI as redirect URI. Can be a space separated list. The redirect uri "
     "must follow the format http://localhost:<port>[/*] or "
     "edu.kit.data.oidc-agent:/<anything>",
     2},
    {"redirect-url", OPT_REDIRECT, "URI", OPTION_ALIAS, NULL, 2},
    {"port", OPT_PORT, "PORT", 0,
     "Use this port in the local redirect uri. Shorter way to pass redirect "
     "uris compared to '--redirect-uri'. Option "
     "can be used multiple times to provide additional backup ports.",
     2},
    {OPT_LONG_OAUTH2, OPT_OAUTH, 0, 0, "Set when using an OAuth2 provider.", 2},
    {"oauth", OPT_OAUTH, 0, OPTION_ALIAS, NULL, 2},

    {0, 0, 0, 0, "Generating a new account configuration - Advanced:", 3},
    {"at", OPT_TOKEN, "ACCESS_TOKEN", 0,
     "Use ACCESS_TOKEN for authorization for authorization at the registration "
     "endpoint.",
     3},
    {"access-token", OPT_TOKEN, "ACCESS_TOKEN", OPTION_ALIAS, NULL, 3},
    {OPT_LONG_AUDIENCE, OPT_AUDIENCE, "AUDIENCE", 0,
     "Limit issued tokens to the specified AUDIENCE. Multiple audiences can be "
     "specified separated by space.",
     3},
    {"audience", OPT_AUDIENCE, "AUDIENCE", OPTION_ALIAS, NULL, 3},
    {OPT_LONG_USERNAME, OPT_USERNAME, "USERNAME", 0,
     "Use USERNAME in the password flow. Requires '--flow=password' to be set.",
     3},
    {OPT_LONG_PASSWORD, OPT_PASSWORD, "PASSWORD", 0,
     "Use PASSWORD in the password flow. Requires '--flow=password' to be set.",
     3},
    {"cnid", OPT_CNID, "IDENTIFIER", 0,
     "Additional identifier used in the client name to distinguish clients on "
     "different machines with the same short name, e.g. the host name",
     3},
    {"client-name-identifier", OPT_CNID, "IDENTIFIER", OPTION_ALIAS, NULL, 3},
    {"cp", OPT_CERTPATH, "FILE", 0,
     "FILE is the path to a CA bundle file that will be used with TLS "
     "communication",
     3},
    {OPT_LONG_CERTPATH, OPT_CERTPATH, "FILE", OPTION_ALIAS, NULL, 3},
    {"cert-file", OPT_CERTPATH, "FILE", OPTION_ALIAS, NULL, 3},
    {OPT_LONG_REFRESHTOKEN, OPT_REFRESHTOKEN, "REFRESH_TOKEN", 0,
     "Use REFRESH_TOKEN as the refresh token in the refresh flow instead of "
     "using another flow. Implicitly sets --flow=refresh",
     3},
    {"refresh-token", OPT_REFRESHTOKEN, "REFRESH_TOKEN", OPTION_ALIAS, NULL, 3},
    {OPT_LONG_REFRESHTOKEN_ENV, OPT_REFRESHTOKEN_ENV,
     OIDC_REFRESHTOKEN_ENV_NAME, OPTION_ARG_OPTIONAL,
     "Like --rt but reads the REFRESH_TOKEN from the passed environment "
     "variable (default: " OIDC_REFRESHTOKEN_ENV_NAME ")",
     3},
    {"refresh-token-env", OPT_REFRESHTOKEN_ENV, OIDC_REFRESHTOKEN_ENV_NAME,
     OPTION_ALIAS, NULL, 3},
    {OPT_LONG_DEVICE, OPT_DEVICE, "ENDPOINT_URI", 0,
     "Use this uri as device authorization endpoint", 3},
    {"device-authorization-endpoint", OPT_DEVICE, "ENDPOINT_URI", OPTION_ALIAS,
     NULL, 3},
    {OPT_LONG_CONFIG_ENDPOINT, OPT_CONFIG_ENDPOINT, "ENDPOINT_URI", 0,
     "Use this uri as the configuration endpoint to read the server's metadata "
     "from",
     3},
    {"config-endpoint", OPT_CONFIG_ENDPOINT, "ENDPOINT_URI", OPTION_ALIAS, NULL,
     3},
    {"discovery-endpoint", OPT_CONFIG_ENDPOINT, "ENDPOINT_URI", OPTION_ALIAS,
     NULL, 3},
    {"flow", 'w', "code|device|password|refresh", 0,
     "Specifies the OIDC flow to be used. Option can be used multiple times to "
     "allow different flows and express priority.",
     3},
    {"only-at", OPT_ONLY_AT, 0, 0,
     "When using this option, oidc-gen will print an access token instead of "
     "creating a new account configuration. No account configuration file is "
     "created. This option does not work with dynamic client registration, but "
     "it does work with preregistered public clients.",
     3},

    {0, 0, 0, 0, "Advanced:", 4},
    {"no-url-call", OPT_NOURLCALL, 0, 0,
     "Does not automatically open the authorization url in a browser.", 4},
    {"pw-cmd", OPT_PW_CMD, "CMD", 0,
     "Command from which oidc-gen can read the encryption password, instead of "
     "prompting the user",
     4},
    {"pw-env", OPT_PW_ENV, OIDC_PASSWORD_ENV_NAME, OPTION_ARG_OPTIONAL,
     "Reads the encryption password from the passed environment variable "
     "(default: " OIDC_PASSWORD_ENV_NAME "), instead of prompting the user",
     4},
    {"pw-file", OPT_PW_FILE, "FILE", 0,
     "Uses the first line of FILE as the encryption password.", 4},
    {"pw-gpg", OPT_PW_GPG, "KEY_ID", 0,
     "Uses the passed GPG KEY for encryption", 4},
    {"pw-pgp", OPT_PW_GPG, "KEY_ID", OPTION_ALIAS, NULL, 4},
    {"gpg", OPT_PW_GPG, "KEY_ID", OPTION_ALIAS, NULL, 4},
    {"pgp", OPT_PW_GPG, "KEY_ID", OPTION_ALIAS, NULL, 4},
    {"pw-prompt", OPT_PW_PROMPT_MODE, "cli|gui", 0,
     "Change the mode how oidc-gen should prompt for passwords. The default is "
     "'cli'.",
     4},
    {"prompt", OPT_PROMPT_MODE, "cli|gui|none", 0,
     "Change the mode how oidc-gen should prompt for information. The default "
     "is 'cli'.",
     4},
    {"confirm-default", OPT_CONFIRM_DEFAULT, 0, 0,
     "Confirms all confirmation prompts with the default value.", 4},
    {"confirm-yes", OPT_CONFIRM_YES, 0, 0,
     "Confirms all confirmation prompts with yes.", 4},
    {"confirm-no", OPT_CONFIRM_NO, 0, 0,
     "Confirms all confirmation prompts with no.", 4},
    {"codeExchange", OPT_codeExchange, "URI", 0,
     "Uses URI to complete the account configuration generation process. URI "
     "must be a full url to which you were redirected after the authorization "
     "code flow.",
     4},
    {"no-webserver", OPT_NO_WEBSERVER, 0, 0,
     "This option applies only when the "
     "authorization code flow is used. oidc-agent will not start a webserver. "
     "Redirection to oidc-gen through a custom uri scheme redirect uri and "
     "'manual' redirect is possible.",
     4},
    {"no-scheme", OPT_NO_SCHEME, 0, 0,
     "This option applies only when the "
     "authorization code flow is used. oidc-agent will not use a custom uri "
     "scheme redirect.",
     4},

    {0, 0, 0, 0, "Internal options:", 5},
    {"state", OPT_state, "STATE", 0,
     "Only for internal usage. Uses STATE to get the associated account config",
     5},

    {0, 0, 0, 0, "Verbosity:", 6},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG", 6},
    {"verbose", 'v', 0, 0, "Enables verbose mode", 6},

    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

/**
 * @brief initializes arguments
 * @param arguments the arguments struct
 */
void initArguments(struct arguments* arguments) {
  arguments->args[0]                       = NULL;
  arguments->print                         = NULL;
  arguments->rename                        = NULL;
  arguments->updateConfigFile              = NULL;
  arguments->codeExchange                  = NULL;
  arguments->state                         = NULL;
  arguments->device_authorization_endpoint = NULL;
  arguments->configuration_endpoint        = NULL;
  arguments->pw_env                        = NULL;
  arguments->pw_cmd                        = NULL;
  arguments->pw_file                       = NULL;
  arguments->pw_gpg                        = NULL;
  arguments->file                          = NULL;

  arguments->client_id     = NULL;
  arguments->client_secret = NULL;
  arguments->issuer        = NULL;
  arguments->redirect_uri  = NULL;
  arguments->scope         = NULL;
  arguments->dynRegToken   = NULL;
  arguments->cert_path     = NULL;
  arguments->refresh_token = NULL;
  arguments->cnid          = NULL;
  arguments->audience      = NULL;
  arguments->op_username   = NULL;
  arguments->op_password   = NULL;

  arguments->flows         = NULL;
  arguments->redirect_uris = NULL;

  arguments->debug           = 0;
  arguments->manual          = 0;
  arguments->verbose         = 0;
  arguments->delete          = 0;
  arguments->listAccounts    = 0;
  arguments->noUrlCall       = 0;
  arguments->usePublicClient = 0;
  arguments->noWebserver     = 0;
  arguments->reauthenticate  = 0;
  arguments->noScheme        = 0;
  arguments->confirm_no      = 0;
  arguments->confirm_yes     = 0;
  arguments->confirm_default = 0;
  arguments->only_at         = 0;
  arguments->noSave          = 0;
  arguments->oauth           = 0;

  arguments->pw_prompt_mode = 0;
  set_pw_prompt_mode(arguments->pw_prompt_mode);
  arguments->prompt_mode = PROMPT_MODE_CLI;
  set_prompt_mode(arguments->prompt_mode);
}

void _setRT(struct arguments* arguments, char* rt) {
  arguments->refresh_token = rt;
  if (arguments->flows == NULL) {
    arguments->flows        = list_new();
    arguments->flows->match = (matchFunction)strequal;
  }
  list_rpush(arguments->flows, list_node_new(FLOW_VALUE_REFRESH));
}

static void _setScope(const char* arg, struct arguments* arguments) {
  if (arguments->scope == NULL) {
    arguments->scope = oidc_strcopy(arg);
    return;
  }
  if (strcaseequal(arguments->scope, AGENT_SCOPE_ALL)) {
    return;
  }
  if (strcaseequal(arg, AGENT_SCOPE_ALL)) {
    secFree(arguments->scope);
    arguments->scope = oidc_strcopy(AGENT_SCOPE_ALL);
    return;
  }
  char* tmp = oidc_sprintf("%s %s", arguments->scope, arg);
  secFree(arguments->scope);
  arguments->scope = tmp;
}

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;

  switch (key) {
    // flags
    case 'd': arguments->delete = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'v': arguments->verbose = 1; break;
    case 'm': arguments->manual = 1; break;
    case OPT_REAUTHENTICATE: arguments->reauthenticate = 1; break;
    case OPT_PUBLICCLIENT: arguments->usePublicClient = 1; break;
    case 'l': arguments->listAccounts = 1; break;
    case OPT_NOURLCALL: arguments->noUrlCall = 1; break;
    case OPT_NO_WEBSERVER: arguments->noWebserver = 1; break;
    case OPT_NO_SCHEME: arguments->noScheme = 1; break;
    case OPT_CONFIRM_NO: arguments->confirm_no = 1; break;
    case OPT_CONFIRM_YES: arguments->confirm_yes = 1; break;
    case OPT_CONFIRM_DEFAULT: arguments->confirm_default = 1; break;
    case OPT_ONLY_AT: arguments->only_at = 1; break;
    case OPT_OAUTH: arguments->oauth = 1; break;
    case OPT_NO_SAVE:
      if (arguments->updateConfigFile) {
        printError("Update argument cannot be combined with no-save\n");
        exit(EXIT_FAILURE);
      }
      if (arguments->rename) {
        printError("Rename argument cannot be combined with no-save\n");
        exit(EXIT_FAILURE);
      }
      arguments->noSave = 1;
      break;

      // arguments
    case 'u':
      if (arguments->noSave) {
        printError("Update argument cannot be combined with no-save\n");
        exit(EXIT_FAILURE);
      }
      arguments->updateConfigFile = arg;
      break;
    case 'p': arguments->print = arg; break;
    case OPT_PW_ENV: arguments->pw_env = arg ?: OIDC_PASSWORD_ENV_NAME; break;
    case OPT_PW_CMD: arguments->pw_cmd = arg; break;
    case OPT_PW_FILE: arguments->pw_file = arg; break;
    case OPT_PW_GPG: arguments->pw_gpg = arg; break;
    case OPT_DEVICE: arguments->device_authorization_endpoint = arg; break;
    case OPT_CONFIG_ENDPOINT: arguments->configuration_endpoint = arg; break;
    case OPT_codeExchange: arguments->codeExchange = arg; break;
    case OPT_state: arguments->state = arg; break;
    case 'f':
      arguments->file   = arg;
      arguments->manual = 1;
      break;
    case OPT_RENAME:
      if (arguments->noSave) {
        printError("Rename argument cannot be combined with no-save\n");
        exit(EXIT_FAILURE);
      }
      arguments->rename = arg;
      break;
    case OPT_PW_PROMPT_MODE:
      if (strequal(arg, "cli")) {
        arguments->pw_prompt_mode = PROMPT_MODE_CLI;
      } else if (strequal(arg, "gui")) {
        arguments->pw_prompt_mode = PROMPT_MODE_GUI;
        common_assertOidcPrompt();
      } else {
        return ARGP_ERR_UNKNOWN;
      }
      set_pw_prompt_mode(arguments->pw_prompt_mode);
      break;
    case OPT_PROMPT_MODE:
      if (strequal(arg, "cli")) {
        arguments->prompt_mode = PROMPT_MODE_CLI;
      } else if (strequal(arg, "gui")) {
        arguments->prompt_mode = PROMPT_MODE_GUI;
        common_assertOidcPrompt();
      } else if (strequal(arg, "none")) {
        arguments->prompt_mode = 0;
      } else {
        return ARGP_ERR_UNKNOWN;
      }
      set_prompt_mode(arguments->prompt_mode);
      break;
    case OPT_TOKEN: arguments->dynRegToken = arg; break;
    case OPT_CERTPATH: arguments->cert_path = arg; break;
    case OPT_REFRESHTOKEN_ENV: {
      const char* env_name          = arg ?: OIDC_REFRESHTOKEN_ENV_NAME;
      char*       env_refresh_token = oidc_strcopy(getenv(env_name));
      if (env_refresh_token == NULL) {
        printError("%s not set!\n", env_name);
        exit(EXIT_FAILURE);
      }
      _setRT(arguments, env_refresh_token);
      break;
    }
    case OPT_REFRESHTOKEN: _setRT(arguments, arg); break;
    case OPT_CNID: arguments->cnid = arg; break;
    case OPT_AUDIENCE: arguments->audience = arg; break;
    case OPT_CLIENTID:
      arguments->client_id = arg;
      arguments->manual    = 1;
      break;
    case OPT_CLIENTSECRET: arguments->client_secret = arg; break;
    case OPT_ISSUER: arguments->issuer = arg; break;
    case OPT_SCOPE: _setScope(arg, arguments); break;
    case OPT_SCOPE_MAX: _setScope(AGENT_SCOPE_ALL, arguments); break;
    case OPT_USERNAME: arguments->op_username = arg; break;
    case OPT_PASSWORD:
      arguments->op_password = arg;
      break;

      // list arguments
    case 'w':
      if (arguments->flows == NULL) {
        arguments->flows        = list_new();
        arguments->flows->match = (matchFunction)strequal;
      }
      list_rpush(arguments->flows, list_node_new(arg));
      break;
    case OPT_PORT:
      if (arguments->redirect_uris == NULL) {
        arguments->redirect_uris        = list_new();
        arguments->redirect_uris->match = (matchFunction)strequal;
        arguments->redirect_uris->free  = _secFree;
      }
      char* redirect_uri = portToUri(strToUShort(arg));
      if (redirect_uri == NULL) {
        oidc_perror();
        exit(EXIT_FAILURE);
      }
      list_rpush(arguments->redirect_uris, list_node_new(redirect_uri));
      break;
    case OPT_REDIRECT:
      if (arguments->redirect_uris == NULL) {
        arguments->redirect_uris = delimitedStringToList(arg, ' ');
        if (checkRedirectUrisForErrors(arguments->redirect_uris) ==
            OIDC_EERROR) {
          exit(EXIT_FAILURE);
        }
      } else {
        list_t* tmp = delimitedStringToList(arg, ' ');
        if (checkRedirectUrisForErrors(tmp) == OIDC_EERROR) {
          exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < tmp->len; i++) {
          list_rpush(arguments->redirect_uris,
                     list_node_new(oidc_strcopy(list_at(tmp, i)->val)));
        }
        secFreeList(tmp);
      }
      break;

    case 'h':
      argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 1 && arguments->delete) {
        argp_usage(state);
      }
      if (arguments->flows == NULL) {
        arguments->flows        = list_new();
        arguments->flows->match = (matchFunction)strequal;
        if (GUIAvailable()) {
          list_rpush(arguments->flows, list_node_new("code"));
        } else {
          list_rpush(arguments->flows, list_node_new("device"));
        }
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "[ACCOUNT_SHORTNAME]";

static char doc[] = "oidc-gen -- A tool for generating oidc account "
                    "configurations which can be used by oidc-add";

struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};
