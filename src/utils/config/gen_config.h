#ifndef OIDC_AGENT_GEN_CONFIG_H
#define OIDC_AGENT_GEN_CONFIG_H

#define CONFIRM_PROMPT_MODE_UNSET 0
#define CONFIRM_PROMPT_MODE_DEFAULT 1
#define CONFIRM_PROMPT_MODE_NO 2
#define CONFIRM_PROMPT_MODE_YES 3

struct gen_config {
  char*         cnid;
  char*         default_gpg_key;
  unsigned char autoopenurl : 1;
  unsigned char prompt_mode : 2;
  unsigned char pw_prompt_mode : 2;
  unsigned char prefer_mytoken_over_oidc : 1;
  unsigned char debug : 1;
  unsigned char answer_confirm_prompts_mode : 2;
  char*         default_mytoken_server;
  char*         default_mytoken_profile;
};

typedef struct gen_config gen_config_t;

const gen_config_t* getGenConfig();

#endif  // OIDC_AGENT_GEN_CONFIG_H
