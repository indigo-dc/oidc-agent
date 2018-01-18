#ifndef OIDC_GEN_H
#define OIDC_GEN_H

#include "version.h"
#include <argp.h>

const char *argp_program_version = GEN_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

struct arguments {
  char* args[1];            /* account */
  int delete;
  int debug;
  int verbose;
  char* file;
  int manual;
  char* output;
  char* codeExchangeRequest;
  char* state;
  char* flow;
};

/* Keys for options without short-options. */
#define OPT_codeExchangeRequest 1
#define OPT_state 2

static struct argp_option options[] = {
  {"delete", 'd', 0, 0, "delete configuration for the given account", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"verbose", 'v', 0, 0, "enables verbose mode. The stored data will be printed.", 0},
  {"file", 'f', "FILE", 0, "specifies file with client config. Implicitly sets -m", 0},
  {"manual", 'm', 0, 0, "Does not use Dynamic Client Registration", 0},
  {"output", 'o', "OUTPUT_FILE", 0, "the path where the client config will be saved", 0},
  {"codeExchangeRequest", OPT_codeExchangeRequest, "REQUEST", OPTION_HIDDEN, "The code Exchange REQUEST", 0},
  {"state", OPT_state, "STATE", OPTION_HIDDEN, "Uses STATE to get the associated account config", 0},
  {"flow", 'w', "FLOW", 0, "Specifies the flow to be used. Multiple space delimited values possible to express priority.", 0},
  {0, 0, 0, 0, 0, 0}
};

/**
 * @brief initializes arguments
 * @param arguments the arguments struct
 */
void initArguments(struct arguments* arguments) {
  arguments->delete = 0;
  arguments->debug = 0;
  arguments->args[0] = NULL;
  arguments->file = NULL;
  arguments->manual = 0;
  arguments->verbose = 0;
  arguments->output = NULL;
  arguments->flow = NULL;
  arguments->codeExchangeRequest = NULL;
  arguments->state = NULL;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key)
  {
    case 'd':
      arguments->delete = 1;
      break;
    case 'g':
      arguments->debug = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'f':
      arguments->file = arg;
      arguments->manual = 1;
      break;
    case 'm':
      arguments->manual = 1;
      break;
    case 'o':
      arguments->output = arg;
      break;
    case OPT_codeExchangeRequest:
      arguments->codeExchangeRequest = arg;
      break;
    case OPT_state:
      arguments->state = arg;
      break;
    case 'w':
      arguments->flow = arg;
      break;
    case ARGP_KEY_ARG:
      if(state->arg_num >= 1) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if(state->arg_num < 1 && arguments->delete) {
        argp_usage (state);
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "[SHORT_NAME]";

static char doc[] = "oidc-gen -- A tool for generating oidc account configurations which can be used by oidc-add";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};



#endif // OIDC_GEN_H
