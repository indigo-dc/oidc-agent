#include "add_config.h"

#include <stddef.h>

#include "configUtils.h"
#include "defines/ipc_values.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/prompting/prompt_mode.h"
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
                 CONFIG_KEY_DEBUGLOGGING, CONFIG_KEY_PLAINADD);
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    SEC_FREE_KEY_VALUES();
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(store_pw, pw_prompt, debug, plainadd);
  add_config_t* c   = secAlloc(sizeof(add_config_t));
  c->store_pw       = strToBit(_store_pw);
  c->pw_prompt_mode = parse_prompt_mode(_pw_prompt);
  c->debug          = strToBit(_debug);
  c->plain_add      = strToBit(_plainadd);
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

  char* add_json = getJSONValue(json, CONFIG_KEY_ADD);
  if (add_json == NULL) {
    _secFreeAddConfig(add_config);
    oidc_perror();
    exit(oidc_errno);
  }
  add_config = _getAddConfig(add_json);
  secFree(add_json);
  return add_config;
}
