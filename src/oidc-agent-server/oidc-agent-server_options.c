#include "oidc-agent-server_options.h"
#include "utils/agentLogger.h"
#include "utils/stringUtils.h"

#define OPT_LOG_CONSOLE 1

void initServerArguments(struct oidcs_arguments* arguments) {
  arguments->kill_flag   = 0;
  arguments->console     = 0;
  arguments->debug       = 0;
  arguments->log_console = 0;
  arguments->data_dir    = "/tmp/oidc-agent-server";
  arguments->port        = 42424;
}

static struct argp_option options[] = {
    {0, 0, 0, 0, "General:", 1},
    {"port", 'p', "PORT", 0,
     "Port number on which the server should be started. Default is 42424.", 1},
    {"data-storage", 's', "PATH", 0,
     "Path to a directory where encrypted configs should be stored. Default is "
     "'/tmp/oidc-agent-server'.",
     1},
    {"kill", 'k', 0, 0,
     "Kill the current agent (given by the OIDCD_PID environment variable)", 1},
    {0, 0, 0, 0, "Verbosity:", 2},
    {"debug", 'g', 0, 0, "Sets the log level to DEBUG.", 2},
    {"console", 'd', 0, 0,
     "Runs oidc-agent on the console, without daemonizing.", 2},
    {"log-stderr", OPT_LOG_CONSOLE, 0, 0,
     "Additionally prints log messages to stderr.", 2},
    {0, 0, 0, 0, "Help:", -1},
    {0, 'h', 0, OPTION_HIDDEN, 0, -1},
    {0, 0, 0, 0, 0, 0}};

static char args_doc[] = "";
static char doc[] = "oidc-agent-server -- An agent running as a central server "
                    "to manage oidc tokens";

static error_t parse_opt(int key, char* arg __attribute__((unused)),
                         struct argp_state* state) {
  struct oidcs_arguments* arguments = state->input;
  switch (key) {
    case 'p':
      arguments->port = strToUShort(arg);
      if (arguments->port == 0) {
        argp_usage(state);
      }
      break;
    case 's': arguments->data_dir = arg; break;
    case 'k': arguments->kill_flag = 1; break;
    case 'g': arguments->debug = 1; break;
    case 'd': arguments->console = 1; break;
    case OPT_LOG_CONSOLE:
      arguments->log_console = 1;
      setLogWithTerminal();
      break;
    case 'h':
      argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
      break;
    case ARGP_KEY_ARG: argp_usage(state); break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

struct argp server_argp = {options, parse_opt, args_doc, doc, 0, 0, 0};
