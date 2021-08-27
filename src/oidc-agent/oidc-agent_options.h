#ifndef OIDC_AGENT_OPTIONS_H
#define OIDC_AGENT_OPTIONS_H

#include <argp.h>

#include "utils/lifetimeArg.h"

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
  unsigned char json;
  unsigned char quiet;

  time_t             lifetime;
  struct lifetimeArg pw_lifetime;

  char* group;
  char* socket_path;
};

void initArguments(struct arguments* arguments);

extern struct argp argp;

#endif  // OIDC_AGENT_OPTIONS_H
