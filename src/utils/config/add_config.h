#ifndef OIDC_AGENT_ADD_CONFIG_H
#define OIDC_AGENT_ADD_CONFIG_H

struct add_config {
  unsigned char store_pw : 1;
  unsigned char pw_prompt_mode : 2;
  unsigned char debug : 1;
};

typedef struct add_config add_config_t;

const add_config_t* getAddConfig();

#endif  // OIDC_AGENT_ADD_CONFIG_H
