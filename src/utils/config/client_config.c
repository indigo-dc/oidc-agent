#include "client_config.h"

#include <stddef.h>

#include "configUtils.h"
#include "defines/ipc_values.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/string/stringUtils.h"

void _secFreeClientConfig(client_config_t* c) {
  if (c == NULL) {
    return;
  }
  secFree(c->default_mytoken_profile);
  secFree(c);
}

static client_config_t* client_config = NULL;

static client_config_t* _getClientConfig(const char* json) {
  if (NULL == json) {
    return secAlloc(sizeof(client_config_t));
  }

  INIT_KEY_VALUE(CONFIG_KEY_DEFAULTMINLIFETIME,
                 CONFIG_KEY_DEFAULTMYTOKENPROFILE);
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    SEC_FREE_KEY_VALUES();
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(default_min_lifetime, default_mytoken_profile);
  client_config_t* c      = secAlloc(sizeof(client_config_t));
  c->default_min_lifetime = strToLong(_default_min_lifetime);
  secFree(_default_min_lifetime);
  c->default_mytoken_profile = _default_mytoken_profile;
  return c;
}

const client_config_t* getClientConfig() {
  if (client_config != NULL) {
    return client_config;
  }
  cJSON* json = readConfig();
  if (json == NULL) {
    client_config = secAlloc(sizeof(client_config_t));
    return client_config;
  }

  INIT_KEY_VALUE(CONFIG_KEY_CLIENT);
  if (getJSONValues((json), pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    SEC_FREE_KEY_VALUES();
    _secFreeClientConfig(client_config);
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(client_config);
  client_config = _getClientConfig(_client_config);
  secFree(_client_config);
  return client_config;
}