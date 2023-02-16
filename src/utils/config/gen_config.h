#ifndef OIDC_AGENT_GEN_CONFIG_H
#define OIDC_AGENT_GEN_CONFIG_H

#define CONFIRM_PROMPT_MODE_UNSET 0
#define CONFIRM_PROMPT_MODE_DEFAULT 1
#define CONFIRM_PROMPT_MODE_NO 2
#define CONFIRM_PROMPT_MODE_YES 3

struct gen_config {
  char*         cnid;
  unsigned char autoopenurl;
  char*         default_gpg_key;
  unsigned char prompt_mode;
  unsigned char pw_prompt_mode;
  char          answer_confirm_prompts_mode;
  char*         default_mytoken_server;
  char*         default_mytoken_profile;
  unsigned char prefer_mytoken_over_oidc;
  unsigned char debug;
};

typedef struct gen_config gen_config_t;

const gen_config_t* getGenConfig();

#endif  // OIDC_AGENT_GEN_CONFIG_H
