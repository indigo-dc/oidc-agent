#ifndef OIDC_AGENT_AGENT_CONFIG_H
#define OIDC_AGENT_AGENT_CONFIG_H

#include <time.h>

struct agent_config {
  char*         cert_path;
  char*         bind_address;
  unsigned char confirm;
  unsigned char autoload;
  unsigned char autoreauth;
  unsigned char customurischeme;
  unsigned char webserver;
  unsigned char debug;
  unsigned char alwaysallowidtoken;
  time_t        lifetime;
  char*         group;
};

typedef struct agent_config agent_config_t;

const agent_config_t* getAgentConfig();

#endif  // OIDC_AGENT_AGENT_CONFIG_H
