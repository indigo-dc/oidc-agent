#ifndef OIDC_AGENT_SERVER_OPTIONS_H
#define OIDC_AGENT_SERVER_OPTIONS_H

#include <argp.h>

struct oidcs_arguments {
  unsigned char kill_flag;
  unsigned char debug;
  unsigned char console;
  unsigned char log_console;

  char*          data_dir;
  unsigned short port;
};

void initServerArguments(struct oidcs_arguments* arguments);

struct argp server_argp;

#endif  // OIDC_AGENT_SERVER_OPTIONS_H
