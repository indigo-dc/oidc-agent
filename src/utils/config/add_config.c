#include "add_config.h"

#include <stddef.h>

#include "configUtils.h"
#include "defines/ipc_values.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/prompt_mode.h"
#include "utils/string/stringUtils.h"

void _secFreeAddConfig(add_config_t* c) {
  if (c == NULL) {
    return;
  }
  secFree(c);
}

static add_config_t* add_config = NULL;

static add_config_t* _getAddConfig(const char* json) {
  if (NULL == json) {
    return secAlloc(sizeof(add_config_t));
  }

  INIT_KEY_VALUE(CONFIG_KEY_STOREPW, CONFIG_KEY_PWPROMPTMODE,
                 CONFIG_KEY_DEBUGLOGGING);
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    SEC_FREE_KEY_VALUES();
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(store_pw, pw_prompt, debug);
  add_config_t* c   = secAlloc(sizeof(add_config_t));
  c->store_pw       = strToUChar(_store_pw);
  c->pw_prompt_mode = parse_prompt_mode(_pw_prompt);
  c->debug          = strToUChar(_debug);
  SEC_FREE_KEY_VALUES();
  return c;
}

const add_config_t* getAddConfig() {
  if (add_config != NULL) {
    return add_config;
  }
  cJSON* json = readConfig();
  if (json == NULL) {
    add_config = secAlloc(sizeof(add_config_t));
    return add_config;
  }

  INIT_KEY_VALUE(CONFIG_KEY_ADD);
  if (getJSONValues((json), pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    SEC_FREE_KEY_VALUES();
    _secFreeAddConfig(add_config);
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(add_json);
  add_config = _getAddConfig(_add_json);
  secFree(_add_json);
  return add_config;
}
