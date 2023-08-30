#include "gen_config.h"

#include <stddef.h>

#include "configUtils.h"
#include "defines/ipc_values.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/printer.h"
#include "utils/prompting/prompt_mode.h"
#include "utils/string/stringUtils.h"

void _secFreeGenConfig(gen_config_t* c) {
  if (c == NULL) {
    return;
  }
  secFree(c->cnid);
  secFree(c->default_mytoken_profile);
  secFree(c->default_mytoken_server);
  secFree(c->default_gpg_key);
  secFree(c);
}

static gen_config_t* gen_config = NULL;

static gen_config_t* _getGenConfig(const char* json) {
  if (NULL == json) {
    return secAlloc(sizeof(gen_config_t));
  }

  INIT_KEY_VALUE(CONFIG_KEY_CNID, CONFIG_KEY_AUTOOPENURL,
                 CONFIG_KEY_DEFAULTGPGKEY, CONFIG_KEY_PROMPTMODE,
                 CONFIG_KEY_PWPROMPTMODE, CONFIG_KEY_ANSWERCONFIRMPROMPTS,
                 CONFIG_KEY_DEFAULTMYTOKENSERVER,
                 CONFIG_KEY_DEFAULTMYTOKENPROFILE,
                 CONFIG_KEY_PREFERMYTOKENOVEROIDC, CONFIG_KEY_DEBUGLOGGING);
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    SEC_FREE_KEY_VALUES();
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(cnid, auto_open_url, default_gpg_key, prompt, pw_prompt,
                 answer_confirm_prompts, default_mytoken_server,
                 default_mytoken_profile, prefer_mytoken_over_oidc, debug);
  gen_config_t* c = secAlloc(sizeof(gen_config_t));
  c->cnid         = _cnid;
  c->autoopenurl  = strToBit(_auto_open_url);
  secFree(_auto_open_url);
  c->default_gpg_key = _default_gpg_key;
  c->prompt_mode     = parse_prompt_mode(_prompt);
  secFree(_prompt);
  c->pw_prompt_mode = parse_prompt_mode(_pw_prompt);
  secFree(_pw_prompt);
  if (strValid(_answer_confirm_prompts)) {
    if (strcaseequal(_answer_confirm_prompts, "yes")) {
      c->answer_confirm_prompts_mode = CONFIRM_PROMPT_MODE_YES;
    } else if (strcaseequal(_answer_confirm_prompts, "no")) {
      c->answer_confirm_prompts_mode = CONFIRM_PROMPT_MODE_NO;
    } else if (strcaseequal(_answer_confirm_prompts, "default")) {
      c->answer_confirm_prompts_mode = CONFIRM_PROMPT_MODE_DEFAULT;
    } else {
      printError("error in oidc-gen config: config attribute '%s' cannot have "
                 "value '%s'\n",
                 CONFIG_KEY_ANSWERCONFIRMPROMPTS, _answer_confirm_prompts);
      exit(EXIT_FAILURE);
    }
  }
  secFree(_answer_confirm_prompts);
  c->default_mytoken_server   = _default_mytoken_server;
  c->default_mytoken_profile  = _default_mytoken_profile;
  c->prefer_mytoken_over_oidc = strToBit(_prefer_mytoken_over_oidc);
  secFree(_prefer_mytoken_over_oidc);
  c->debug = strToBit(_debug);
  secFree(_debug);
  return c;
}

const gen_config_t* getGenConfig() {
  if (gen_config != NULL) {
    return gen_config;
  }
  cJSON* json = readConfig();
  if (json == NULL) {
    gen_config = secAlloc(sizeof(gen_config_t));
    return gen_config;
  }

  char* gen_json = getJSONValue(json, CONFIG_KEY_GEN);
  if (gen_json == NULL) {
    _secFreeGenConfig(gen_config);
    oidc_perror();
    exit(oidc_errno);
  }
  gen_config = _getGenConfig(gen_json);
  secFree(gen_json);
  return gen_config;
}
