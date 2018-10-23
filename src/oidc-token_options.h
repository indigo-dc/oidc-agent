#ifndef OIDC_TOKEN_OPTIONS_H
#define OIDC_TOKEN_OPTIONS_H

#include <argp.h>
#include <stdlib.h>

struct arguments {
  char*         args[1]; /* account shortname */
  int           list_accounts;
  unsigned long min_valid_period;
  char*         scope;
};

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"listaccounts", 'l', 0, 0, "Lists the currently loaded accounts", 1},
    {"time", 't', "SECONDS", 0,
     "Minimum number of seconds the access token should be valid", 1},
    {0, 0, 0, 0, "Advanced:", 2},
    {"scope", 's', "SCOPE", 0,
     "Space delimited list of scopes to be requested for the requested access "
     "token",
     2},
    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;

  switch (key) {
    case 'l': arguments->list_accounts = 1; break;
    case 's': arguments->scope = arg; break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->min_valid_period = atoi(arg);
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
      if (arguments->list_accounts) {
        break;
      }
      if (state->arg_num < 1) {
        argp_usage(state);
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "ACCOUNT_SHORTNAME | -l";

static char doc[] =
    "oidc-token -- A client for oidc-agent for getting OIDC access tokens.";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

static void initArguments(struct arguments* arguments) {
  arguments->min_valid_period = 0;
  arguments->list_accounts    = 0;
  arguments->args[0]          = NULL;
  arguments->scope            = NULL;
}

#endif  // OIDC_TOKEN_OPTIONS_H
