#include "agent_config.h"

#include <stddef.h>

#include "configUtils.h"
#include "defines/ipc_values.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/string/stringUtils.h"

void _secFreeAgentConfig(agent_config_t* c) {
  if (c == NULL) {
    return;
  }
  secFree(c->cert_path);
  secFree(c->bind_address);
  secFree(c->group);
  secFree(c);
}

static agent_config_t* agent_config = NULL;

static agent_config_t* _getAgentConfig(const char* json) {
  if (NULL == json) {
    return secAlloc(sizeof(agent_config_t));
  }

  INIT_KEY_VALUE(AGENT_KEY_CERTPATH, CONFIG_KEY_BINDADDRESS, CONFIG_KEY_CONFIRM,
                 CONFIG_KEY_AUTOLOAD, CONFIG_KEY_AUTOREAUTH,
                 CONFIG_KEY_CUSTOMURISCHEME, CONFIG_KEY_WEBSERVER,
                 CONFIG_KEY_DEBUGLOGGING, IPC_KEY_LIFETIME, CONFIG_KEY_GROUP,
                 IPC_KEY_ALWAYSALLOWID);
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    SEC_FREE_KEY_VALUES();
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(cert_path, bind_address, confirm, autoload, autoreauth,
                 customurischeme, webserver, debug, lifetime, group,
                 alwaysallowidtoken);
  agent_config_t* c     = secAlloc(sizeof(agent_config_t));
  c->cert_path          = oidc_strcopy(_cert_path);
  c->bind_address       = oidc_strcopy(_bind_address);
  c->group              = oidc_strcopy(_group);
  c->confirm            = strToUChar(_confirm);
  c->autoload           = strToUChar(_autoload);
  c->autoreauth         = strToUChar(_autoreauth);
  c->customurischeme    = strToUChar(_customurischeme);
  c->webserver          = strToUChar(_webserver);
  c->alwaysallowidtoken = strToUChar(_alwaysallowidtoken);
  c->debug              = strToUChar(_debug);
  c->lifetime           = strToLong(_lifetime);
  SEC_FREE_KEY_VALUES();
  return c;
}

const agent_config_t* getAgentConfig() {
  if (agent_config != NULL) {
    return agent_config;
  }
  cJSON* json = readConfig();
  if (json == NULL) {
    agent_config = secAlloc(sizeof(agent_config_t));
    return agent_config;
  }

  INIT_KEY_VALUE(CONFIG_KEY_AGENT);
  if (getJSONValues((json), pairs, sizeof(pairs) / sizeof(*pairs)) < 0) {
    SEC_FREE_KEY_VALUES();
    _secFreeAgentConfig(agent_config);
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(agent_json);
  agent_config = _getAgentConfig(_agent_json);
  secFree(_agent_json);
  return agent_config;
}
