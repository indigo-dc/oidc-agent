#ifndef OIDC_AGENT_OPTIONS_H
#define OIDC_AGENT_OPTIONS_H

#include "utils/lifetimeArg.h"

#include <argp.h>

struct arguments {
  unsigned char kill_flag;
  unsigned char debug;
  unsigned char console;
  unsigned char seccomp;
  unsigned char no_autoload;
  unsigned char confirm;
  unsigned char no_webserver;
  unsigned char no_scheme;
  unsigned char always_allow_idtoken;
  unsigned char log_console;
  unsigned char status;

  time_t             lifetime;
  struct lifetimeArg pw_lifetime;

  char* group;
};

void initArguments(struct arguments* arguments);

struct argp argp;

#endif  // OIDC_AGENT_OPTIONS_H
