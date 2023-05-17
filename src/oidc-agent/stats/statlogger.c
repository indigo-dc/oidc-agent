#define _XOPEN_SOURCE
#include "statlogger.h"

#ifndef NO_STATLOG
#include <string.h>
#include <time.h>

#include "defines/ipc_values.h"
#include "oidc-agent/http/http_ipc.h"
#include "statid.h"
#include "utils/config/agent_config.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/key_value.h"
#include "utils/listUtils.h"
#include "utils/string/stringUtils.h"

static unsigned char called_tz_set = 0;

static char* createMsg(const char* ipc_request) {
  if (!called_tz_set) {
    tzset();
    called_tz_set = 1;
  }
  INIT_KEY_VALUE(IPC_KEY_REQUEST, IPC_KEY_SHORTNAME, IPC_KEY_MINVALID,
                 IPC_KEY_CONFIG, IPC_KEY_FLOW, OIDC_KEY_SCOPE, IPC_KEY_LIFETIME,
                 IPC_KEY_APPLICATIONHINT, IPC_KEY_ISSUERURL, IPC_KEY_AUDIENCE,
                 IPC_KEY_ONLYAT, AGENT_KEY_MYTOKENPROFILE,
                 AGENT_KEY_CONFIG_ENDPOINT, INT_IPC_KEY_ACTION);
  if (getJSONValuesFromString(ipc_request, pairs,
                              sizeof(pairs) / sizeof(*pairs)) < 0) {
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    return NULL;
  }
  KEY_VALUE_VARS(request, shortname, minvalid, config, flow, scope, lifetime,
                 applicationHint, issuer, audience, only_at, profile,
                 config_endpoint, action);
  char* mytoken_url = NULL;
  if (_config != NULL) {
    if (_issuer == NULL) {
      _issuer = getJSONValueFromString(_config, OIDC_KEY_ISSUER);
      if (_issuer == NULL) {
        _issuer = getJSONValueFromString(_config, AGENT_KEY_ISSUERURL);
      }
    }
    if (_shortname == NULL) {
      _shortname = getJSONValueFromString(_config, AGENT_KEY_SHORTNAME);
    }
    mytoken_url = getJSONValueFromString(_config, AGENT_KEY_MYTOKENURL);
  }
  const struct statid id   = getStatID();
  cJSON*              json = generateJSONObject(
      "machine_id", cJSON_String, id.machine_id, "boot_id", cJSON_String,
      id.boot_id, "os_info", cJSON_String, id.os_info, "time", cJSON_Number,
      time(NULL), "lt_offset", cJSON_Number, (daylight ? 3600 : 0) - timezone,
      "agent_version", cJSON_String, VERSION, NULL);

  if (id.location) {
    jsonAddStringValue(json, "country", id.location);
  }
  if (_request) {
    jsonAddStringValue(json, IPC_KEY_REQUEST, _request);
  }
  if (_issuer) {
    jsonAddStringValue(json, IPC_KEY_ISSUERURL, _issuer);
  }
  if (_config_endpoint) {
    jsonAddStringValue(json, AGENT_KEY_CONFIG_ENDPOINT, _config_endpoint);
  }
  if (mytoken_url) {
    jsonAddStringValue(json, AGENT_KEY_MYTOKENURL, mytoken_url);
  }
  if (_shortname) {
    jsonAddStringValue(json, IPC_KEY_SHORTNAME, _shortname);
  }
  if (_applicationHint) {
    jsonAddStringValue(json, IPC_KEY_APPLICATIONHINT, _applicationHint);
  }
  if (_flow) {
    jsonAddStringValue(json, IPC_KEY_FLOW, _flow);
  }
  if (_scope) {
    jsonAddStringValue(json, OIDC_KEY_SCOPE, _scope);
  }
  if (_audience) {
    jsonAddStringValue(json, IPC_KEY_AUDIENCE, _audience);
  }
  if (_minvalid) {
    jsonAddNumberValue(json, IPC_KEY_MINVALID, strToInt(_minvalid));
  }
  if (_lifetime) {
    jsonAddNumberValue(json, IPC_KEY_LIFETIME, strToInt(_lifetime));
  }
  if (_only_at) {
    jsonAddBoolValue(json, IPC_KEY_ONLYAT, strToInt(_only_at));
  }
  if (_profile) {
    jsonAddStringValue(json, AGENT_KEY_MYTOKENPROFILE, _profile);
  }
  if (_action) {
    jsonAddStringValue(json, INT_IPC_KEY_ACTION, _action);
  }
  SEC_FREE_KEY_VALUES();
  char* msg = jsonToStringUnformatted(json);
  secFreeJson(json);
  return msg;
}

static time_t lastSendTime = 0;
#define ONE_DAY 86400

static size_t charPos = 0;

#ifndef SYNC_BLOCK
#define SYNC_BLOCK "### SYNC BLOCK ###"
#endif
#ifndef STATS_FILE
#define STATS_FILE "oidc-agent.stats"
#endif
#ifndef STATS_SERVER
#define STATS_SERVER "https://oidc-agent.test.fedcloud.eu"
#endif

static oidc_error_t sendStatPayload(const char* payload) {
  char* res = sendJSONPostWithoutBasicAuth(STATS_SERVER, payload, NULL, NULL);
  oidc_error_t ret = strcaseequal(res, "ok") ? OIDC_SUCCESS : OIDC_EERROR;
  secFree(res);
  return ret;
}

static void sendStats() {
  if (!getAgentConfig()->stats_collect_share) {
    return;
  }
  time_t now = time(NULL);
  if (now < lastSendTime + ONE_DAY) {
    return;
  }
  char* new_stats = getFileContentFromOidcFileAfterLine(STATS_FILE, SYNC_BLOCK,
                                                        (long)charPos, 1);
  if (!strValid(new_stats)) {
    secFree(new_stats);
    return;
  }
  if (strCountChar(new_stats, '\n') < 10) {
    secFree(new_stats);
    return;
  }
  char* jsonStats = delimitedStringToJSONArray(new_stats, '\n');
  if (sendStatPayload(jsonStats) == OIDC_SUCCESS) {
    charPos += strlen(new_stats);
    lastSendTime = now;
    appendOidcFile(STATS_FILE, SYNC_BLOCK);
  }
  secFree(jsonStats);
  secFree(new_stats);
}

#endif  // NO_STATLOG

void statlog(const char* ipc_request) {
#ifndef NO_STATLOG
  if (!getAgentConfig()->stats_collect) {
    return;
  }
  char* msg = createMsg(ipc_request);
  appendOidcFile(STATS_FILE, msg);
  sendStats();
#endif  // NO_STATLOG
}
