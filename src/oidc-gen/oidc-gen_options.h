#ifndef OIDC_GEN_OPTIONS_H
#define OIDC_GEN_OPTIONS_H

#include "list/list.h"
#include "utils/stringUtils.h"

#include <argp.h>

struct optional_arg {
  char* str;
  short useIt;
};

struct arguments {
  char* args[1]; /* account */
  int delete;
  int                 debug;
  int                 verbose;
  char*               file;
  int                 manual;
  char*               output;
  char*               codeExchangeRequest;
  char*               state;
  list_t*             flows;
  int                 listClients;
  int                 listAccounts;
  char*               print;
  struct optional_arg dynRegToken;
  struct optional_arg cert_path;
  struct optional_arg refresh_token;
  char*               client_name_id;
  int                 qr;
  int                 qrterminal;
  char*               device_authorization_endpoint;
  int                 splitConfigFiles;
  int                 noSeccomp;
  int                 _nosec;
  int                 noUrlCall;
};

/* Keys for options without short-options. */
#define OPT_codeExchangeRequest 1
#define OPT_state 2
#define OPT_TOKEN 3
#define OPT_CERTPATH 4
#define OPT_QR 5
#define OPT_QRTERMINAL 6
#define OPT_DEVICE 7
#define OPT_CNID 8
#define OPT_NOSECCOMP 9
#define OPT_NOURLCALL 10
#define OPT_REFRESHTOKEN 11

static struct argp_option options[] = {

    {0, 0, 0, 0, "Getting information:", 1},
    {"accounts", 'l', 0, 0,
     "Prints a list of available account configurations. Same as oidc-add -l",
     1},
    {"clients", 'c', 0, 0, "Prints a list of available client configurations",
     1},
    {"print", 'p', "FILE", 0,
     "Prints the decrypted content of FILE. FILE can be an absolute path or "
     "the name of a file placed in oidc-dir (e.g. an account configuration "
     "short name)",
     1},

    {0, 0, 0, 0, "Generating a new account configuration:", 2},
    {"file", 'f', "FILE", 0,
     "Reads the client configuration from FILE. Implicitly sets -m", 2},
    {"manual", 'm', 0, 0,
     "Does not use Dynamic Client Registration. Client has to be manually "
     "registered beforehand",
     2},
    {"delete", 'd', 0, 0, "Delete configuration for the given account", 2},
    {"at", OPT_TOKEN, "ACCESS_TOKEN", OPTION_ARG_OPTIONAL,
     "An access token used for authorization if the registration endpoint is "
     "protected",
     2},

    {0, 0, 0, 0, "Advanced:", 3},
    {"output", 'o', "OUTPUT_FILE", 0,
     "When using Dynamic Client Registration the resulting client "
     "configuration will be stored in OUTPUT_FILE instead of inside the "
     "oidc-agent directory. Implicitly sets the -s option.",
     3},
    {"cp", OPT_CERTPATH, "CERT_PATH", OPTION_ARG_OPTIONAL,
     "CERT_PATH is the path to a CA bundle file that will be used with TLS "
     "communication",
     3},
    {"rt", OPT_REFRESHTOKEN, "REFRESH_TOKEN", OPTION_ARG_OPTIONAL,
     "Use the specified REFRESH_TOKEN with the refresh flow instead of using "
     "another flow. Implicitly sets --flow=refresh",
     3},
    {"flow", 'w', "FLOW", 0,
     "Specifies the OIDC flow to be used. Option can be used multiple times to "
     "allow different flows and express priority. Possible values are: code "
     "device password refresh",
     3},
    {"qr", OPT_QR, 0, 0,
     "When using the device flow a QR-Code containing the device uri is "
     "printed",
     3},
    {"qrt", OPT_QRTERMINAL, 0, 0,
     "When using the device flow a QR-Code containing the device uri is "
     "printed directly to the terminal. Implicitly sets --qr",
     3},
    {"dae", OPT_DEVICE, "ENDPOINT_URI", 0,
     "Use this uri as device authorization endpoint", 3},
    {"cnid", OPT_CNID, "CLIENTNAME__IDENTIFIER", 0,
     "Additional identifier used in the client name to distinguish clients on "
     "diferent machines with the same short name, e.g. the host name",
     3},
    {"split-config", 's', 0, 0,
     "Use separate configuration files for the registered client and the "
     "account configuration.",
     3},
    {"no-seccomp", OPT_NOSECCOMP, 0, 0,
     "Disables seccomp system call filtering; allowing all system calls. Use "
     "this option if you get an 'Bad system call' error and hand in a bug "
     "report.",
     3},
    {"no-url-call", OPT_NOURLCALL, 0, 0,
     "Does not automatically open the authorization url in a browser. Enables "
     "oidc-gen to use seccomp.",
     3},

    {0, 0, 0, 0, "Internal options:", 4},
    {"codeExchangeRequest", OPT_codeExchangeRequest, "REQUEST", 0,
     "Only for internal usage. Performs a code exchange request with REQUEST",
     4},
    {"state", OPT_state, "STATE", 0,
     "Only for internal usage. Uses STATE to get the associated account config",
     4},

    {0, 0, 0, 0, "Verbosity:", 5},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG", 5},
    {"verbose", 'v', 0, 0, "Enables verbose mode", 5},

    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

/**
 * @brief initializes arguments
 * @param arguments the arguments struct
 */
static inline void initArguments(struct arguments* arguments) {
  arguments->delete                        = 0;
  arguments->debug                         = 0;
  arguments->args[0]                       = NULL;
  arguments->file                          = NULL;
  arguments->manual                        = 0;
  arguments->verbose                       = 0;
  arguments->output                        = NULL;
  arguments->flows                         = NULL;
  arguments->codeExchangeRequest           = NULL;
  arguments->state                         = NULL;
  arguments->listClients                   = 0;
  arguments->listAccounts                  = 0;
  arguments->print                         = NULL;
  arguments->dynRegToken.str               = NULL;
  arguments->dynRegToken.useIt             = 0;
  arguments->refresh_token.str             = NULL;
  arguments->refresh_token.useIt           = 0;
  arguments->cert_path.str                 = NULL;
  arguments->cert_path.useIt               = 0;
  arguments->client_name_id                = NULL;
  arguments->qr                            = 0;
  arguments->qrterminal                    = 0;
  arguments->device_authorization_endpoint = NULL;
  arguments->splitConfigFiles              = 0;
  arguments->noSeccomp                     = 0;
  arguments->_nosec                        = 0;
  arguments->noUrlCall                     = 0;
}

static inline error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;

  switch (key) {
    case 'd': arguments->delete = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'v': arguments->verbose = 1; break;
    case 'f':
      arguments->file   = arg;
      arguments->manual = 1;
      break;
    case 'm': arguments->manual = 1; break;
    case 'o':
      arguments->output           = arg;
      arguments->splitConfigFiles = 1;
      break;
    case OPT_codeExchangeRequest: arguments->codeExchangeRequest = arg; break;
    case OPT_state: arguments->state = arg; break;
    case OPT_TOKEN:
      arguments->dynRegToken.str   = arg;
      arguments->dynRegToken.useIt = 1;
      break;
    case OPT_CERTPATH:
      arguments->cert_path.str   = arg;
      arguments->cert_path.useIt = 1;
      break;
    case OPT_REFRESHTOKEN:
      arguments->refresh_token.str   = arg;
      arguments->refresh_token.useIt = 1;
      if (arguments->flows == NULL) {
        arguments->flows        = list_new();
        arguments->flows->match = (int (*)(void*, void*))strequal;
      }
      list_rpush(arguments->flows, list_node_new("refresh"));
      break;
    case OPT_CNID: arguments->client_name_id = arg; break;
    case OPT_QR: arguments->qr = 1; break;
    case OPT_QRTERMINAL:
      arguments->qr         = 1;
      arguments->qrterminal = 1;
      arguments->noSeccomp  = 1;
      break;
    case OPT_DEVICE: arguments->device_authorization_endpoint = arg; break;
    case 'w':
      if (arguments->flows == NULL) {
        arguments->flows        = list_new();
        arguments->flows->match = (int (*)(void*, void*))strequal;
      }
      list_rpush(arguments->flows, list_node_new(arg));
      if (strSubStringCase(arg, "code")) {
        arguments->_nosec = 1;
      }
      break;
    case 'l': arguments->listAccounts = 1; break;
    case 'c': arguments->listClients = 1; break;
    case 'p': arguments->print = arg; break;
    case 's': arguments->splitConfigFiles = 1; break;
    case OPT_NOSECCOMP: arguments->noSeccomp = 1; break;
    case OPT_NOURLCALL: arguments->noUrlCall = 1; break;
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
        arguments->flows->match = (int (*)(void*, void*))strequal;
        list_rpush(arguments->flows, list_node_new("code"));
        arguments->_nosec = 1;
      }
      if (arguments->_nosec && !arguments->noUrlCall) {
        arguments->noSeccomp = 1;
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "[SHORT_NAME]";

static char doc[] = "oidc-gen -- A tool for generating oidc account "
                    "configurations which can be used by oidc-add";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

#endif  // OIDC_GEN_OPTIONS_H
