#ifndef OIDC_AGENT_H
#define OIDC_AGENT_H

#include "version.h"

#include <argp.h>

const char *argp_program_version = AGENT_VERSION;

const char *argp_program_bug_address = BUG_ADDRESS;

struct arguments {
  int kill_flag;
  int debug;
  int console;
};

static struct argp_option options[] = {
  {"kill", 'k', 0, 0, "Kill the current agent (given by the OIDCD_PID environment variable).", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"console", 'c', 0, 0, "runs oidc-agent on the console, without daemonizing", 0},
  {0, 0, 0, 0, 0, 0}
};

static char args_doc[] = "";
static char doc[] = "oidc-agent -- A agent to manage oidc token";

static error_t parse_opt (int key, char *arg __attribute__((unused)), struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 'k':
      arguments->kill_flag = 1;
      break;
    case 'g':
      arguments->debug = 1;
      break;
    case 'c':
      arguments->console = 1;
      break;
    case ARGP_KEY_ARG:
      argp_usage(state);
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

#endif //OIDC_AGENT_H
