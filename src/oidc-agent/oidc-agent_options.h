#ifndef OIDC_AGENT_OPTIONS_H
#define OIDC_AGENT_OPTIONS_H

#include "utils/stringUtils.h"

#include <argp.h>
#include <time.h>

struct arguments {
  int    kill_flag;
  int    debug;
  int    console;
  time_t lifetime;
  int    seccomp;
};

#define OPT_SECCOMP 1

static inline void initArguments(struct arguments* arguments) {
  arguments->kill_flag = 0;
  arguments->console   = 0;
  arguments->debug     = 0;
  arguments->lifetime  = 0;
  arguments->seccomp   = 0;
}

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"kill", 'k', 0, 0,
     "Kill the current agent (given by the OIDCD_PID environment variable)", 1},
    {"lifetime", 't', "LIFETIME", 0,
     "Set a default value in seconds for the maximum lifetime of account "
     "configurations added to the agent. A lifetime specified for an account "
     "configuration with oidc-add overwrites this default value. Without this "
     "option the default maximum lifetime is forever.",
     1},
    {"seccomp", OPT_SECCOMP, 0, 0,
     "Enables seccomp system call filtering; allowing only predefined system "
     "calls.",
     1},
    {0, 0, 0, 0, "Verbosity:", 2},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG", 2},
    {"console", 'c', 0, 0,
     "Runs oidc-agent on the console, without daemonizing", 2},
    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

static char args_doc[] = "";
static char doc[]      = "oidc-agent -- An agent to manage oidc token";

static error_t parse_opt(int key, char* arg __attribute__((unused)),
                         struct argp_state* state) {
  struct arguments* arguments = state->input;
  switch (key) {
    case 'k': arguments->kill_flag = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'c': arguments->console = 1; break;
    case OPT_SECCOMP: arguments->seccomp = 1; break;
    case 't':
      if (!isdigit(*arg)) {
        return ARGP_ERR_UNKNOWN;
      }
      arguments->lifetime = strToInt(arg);
      break;
    case 'h':
      argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case ARGP_KEY_ARG: argp_usage(state);
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

#endif  // OIDC_AGENT_OPTIONS_H
