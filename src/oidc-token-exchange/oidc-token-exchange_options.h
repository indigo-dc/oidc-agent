#ifndef OIDC_EXCHANGE_OPTIONS_H
#define OIDC_EXCHANGE_OPTIONS_H

#include <argp.h>
#include <stdlib.h>
#include <time.h>

struct lifetimeArg {
  time_t lifetime;
  short  argProvided;
};

struct arguments {
  char*              args[4]; /* account client_id client_secret access_token */
  int                remove;
  int                debug;
  int                verbose;
  struct lifetimeArg lifetime;
  char*              cert_path;
  int                noSeccomp;
};

/* Keys for options without short-options. */
#define OPT_CERTPATH 1
#define OPT_NOSECCOMP 2

static struct argp_option options[] = {

    {0, 0, 0, 0, "General Usage:", 1},
    {"revoke", 'r', 0, 0,
     "Removes an account from the agent and revokes the associated refresh "
     "token.",
     1},
    {"lifetime", 't', "LIFETIME", 0,
     "Set a maximum lifetime in seconds when adding the account configuration",
     1},

    {0, 0, 0, 0, "Advanced:", 2},
    {"cp", OPT_CERTPATH, "CERT_PATH", 0,
     "CERT_PATH is the path to a CA bundle file that will be used with TLS "
     "communication",
     2},
    {"no-seccomp", OPT_NOSECCOMP, 0, 0,
     "Disables seccomp system call filtering; allowing all system calls. Use "
     "this option if you get an 'Bad system call' error and hand in a bug "
     "report.",
     2},

    {0, 0, 0, 0, "Verbosity:", 3},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG", 3},
    {"verbose", 'v', 0, 0, "Enables verbose mode", 3},

    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

/**
 * @brief initializes arguments
 * @param arguments the arguments struct
 */
static inline void initArguments(struct arguments* arguments) {
  arguments->remove    = 0;
  arguments->debug     = 0;
  arguments->args[0]   = NULL;
  arguments->args[1]   = NULL;
  arguments->args[2]   = NULL;
  arguments->args[3]   = NULL;
  arguments->verbose   = 0;
  arguments->cert_path = NULL;
  arguments->noSeccomp = 0;
}

static inline error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;

  switch (key) {
    case 'r': arguments->remove = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'v': arguments->verbose = 1; break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->lifetime.lifetime    = atoi(arg);
      arguments->lifetime.argProvided = 1;
      break;
    case OPT_CERTPATH: arguments->cert_path = arg; break;
    case OPT_NOSECCOMP: arguments->noSeccomp = 1; break;
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
      if (state->arg_num > 1 && state->arg_num < 4) {
        argp_usage(state);
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "SHORT_NAME [CLIENT_ID CLIENT_SECRET ACCESS_TOKEN]";

static char doc[] = "oidc-token-exchange -- A tool for performing OIDC token "
                    "exchanges using oidc-agent";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

#endif  // OIDC_EXCHANGE_OPTIONS_H
