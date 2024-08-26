#include "agent_config.h"

#include <stddef.h>

#include "configUtils.h"
#include "defines/ipc_values.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/printer.h"
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
                 IPC_KEY_ALWAYSALLOWID, CONFIG_KEY_AUTOGEN,
                 CONFIG_KEY_AUTOGENSCOPEMODE, CONFIG_KEY_STATSCOLLECT,
                 CONFIG_KEY_STATSCOLLECTSHARE, CONFIG_KEY_STATSCOLLECTLOCATION,
                 CONFIG_KEY_RESTARTAFTERUPDATE);
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    SEC_FREE_KEY_VALUES();
    oidc_perror();
    exit(oidc_errno);
  }
  KEY_VALUE_VARS(cert_path, bind_address, confirm, autoload, autoreauth,
                 customurischeme, webserver, debug, lifetime, group,
                 alwaysallowidtoken, autogen, autogenscopemode, stats_collect,
                 stats_collect_share, stats_collect_location,
                 restart_after_update);
  agent_config_t* c         = secAlloc(sizeof(agent_config_t));
  c->cert_path              = oidc_strcopy(_cert_path);
  c->bind_address           = oidc_strcopy(_bind_address);
  c->group                  = oidc_strcopy(_group);
  c->confirm                = strToBit(_confirm);
  c->autoload               = strToBit(_autoload);
  c->autoreauth             = strToBit(_autoreauth);
  c->customurischeme        = strToBit(_customurischeme);
  c->webserver              = strToBit(_webserver);
  c->alwaysallowidtoken     = strToBit(_alwaysallowidtoken);
  c->autogen                = strToBit(_autogen);
  c->stats_collect          = strToBit(_stats_collect);
  c->stats_collect_share    = strToBit(_stats_collect_share);
  c->stats_collect_location = strToBit(_stats_collect_location);
  c->restart_after_update   = strValid(_restart_after_update)
                                  ? strToBit(_restart_after_update)
                                  : 1;  // default is on
  if (strValid(_autogenscopemode)) {
    if (strcaseequal(_autogenscopemode, CONFIG_VALUE_SCOPEMODE_EXACT)) {
      c->autogenscopemode = AGENTCONFIG_AUTOGENSCOPEMODE_EXACT;
    } else if (strcaseequal(_autogenscopemode, CONFIG_VALUE_SCOPEMODE_MAX)) {
      c->autogenscopemode = AGENTCONFIG_AUTOGENSCOPEMODE_ALL;
    } else {
      printError(
          "error in oidc-agent config: config attribute '%s' cannot have "
          "value '%s'\n",
          CONFIG_KEY_AUTOGENSCOPEMODE, _autogenscopemode);
      SEC_FREE_KEY_VALUES();
      exit(EXIT_FAILURE);
    }
  }
  c->debug    = strToBit(_debug);
  c->lifetime = strToLong(_lifetime);
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

  char* agent_json = getJSONValue(json, CONFIG_KEY_AGENT);
  secFreeJson(json);
  if (agent_json == NULL) {
    _secFreeAgentConfig(agent_config);
    oidc_perror();
    exit(oidc_errno);
  }
  agent_config = _getAgentConfig(agent_json);
  secFree(agent_json);
  return agent_config;
}
