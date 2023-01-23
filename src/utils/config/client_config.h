#ifndef OIDC_AGENT_CLIENT_CONFIG_H
#define OIDC_AGENT_CLIENT_CONFIG_H

#include <time.h>

struct client_config {
  time_t default_min_lifetime;
  char*  default_mytoken_profile;
};

typedef struct client_config client_config_t;

const client_config_t* getClientConfig();

#endif  // OIDC_AGENT_CLIENT_CONFIG_H
