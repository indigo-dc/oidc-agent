#ifndef OIDC_AGENT_ADD_CONFIG_H
#define OIDC_AGENT_ADD_CONFIG_H

struct add_config {
  unsigned char store_pw;
  unsigned char pw_prompt_mode;
  unsigned char debug;
};

typedef struct add_config add_config_t;

const add_config_t* getAddConfig();

#endif  // OIDC_AGENT_ADD_CONFIG_H
