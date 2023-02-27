#ifndef OIDC_AGENT_AGENT_CONFIG_H
#define OIDC_AGENT_AGENT_CONFIG_H

#include <time.h>

#define AGENTCONFIG_AUTOGENSCOPEMODE_ALL 0
#define AGENTCONFIG_AUTOGENSCOPEMODE_EXACT 1
#define AGENTCONFIG_AUTOGENSCOPEMODE_RESERVED 2
#define AGENTCONFIG_AUTOGENSCOPEMODE_RESERVED_2 3

struct agent_config {
  char*         cert_path;
  char*         bind_address;
  unsigned char confirm : 1;
  unsigned char autoload : 1;
  unsigned char autoreauth : 1;
  unsigned char customurischeme : 1;
  unsigned char webserver : 1;
  unsigned char debug : 1;
  unsigned char alwaysallowidtoken : 1;
  unsigned char autogen : 1;
  unsigned char autogenscopemode : 2;
  time_t        lifetime;
  char*         group;
};

typedef struct agent_config agent_config_t;

const agent_config_t* getAgentConfig();

#endif  // OIDC_AGENT_AGENT_CONFIG_H
