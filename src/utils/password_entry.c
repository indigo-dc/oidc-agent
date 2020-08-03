#include "password_entry.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"

void _secFreePasswordEntry(struct password_entry* pw) {
  secFree(pw->shortname);
  secFree(pw->password);
  secFree(pw->command);
  secFree(pw);
}

void pwe_setPassword(struct password_entry* pw, char* password) {
  if (pw->password == password) {
    return;
  }
  secFree(pw->password);
  pw->password = password;
  logger(DEBUG, "Setting password. Expires_at is %lu. Expires after is %lu",
         pw->expires_at, pw->expires_after);
  if (pw->expires_at == 0 && pw->expires_after != 0) {
    pw->expires_at = time(NULL) + pw->expires_after;
  }
}

void pwe_setCommand(struct password_entry* pw, char* command) {
  if (pw->command == command) {
    return;
  }
  secFree(pw->command);
  pw->command = command;
}

void pwe_setShortname(struct password_entry* pw, char* shortname) {
  if (pw->shortname == shortname) {
    return;
  }
  secFree(pw->shortname);
  pw->shortname = shortname;
}

void pwe_setType(struct password_entry* pw, unsigned char type) {
  pw->type = type;
}

void pwe_setExpiresAt(struct password_entry* pw, time_t expires_at) {
  pw->expires_at = expires_at;
}

void pwe_setExpiresIn(struct password_entry* pw, time_t expires_in) {
  pwe_setExpiresAt(pw, expires_in ? time(NULL) + expires_in : 0);
  pwe_setExpiresAfter(pw, expires_in);
}

void pwe_setExpiresAfter(struct password_entry* pw, time_t expires_after) {
  pw->expires_after = expires_after;
}

cJSON* passwordEntryToJSON(const struct password_entry* pw) {
  if (pw == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* json = stringToJson("{}");
  if (pw->shortname) {
    jsonAddStringValue(json, PW_KEY_SHORTNAME, pw->shortname);
  }
  if (pw->type) {
    jsonAddNumberValue(json, PW_KEY_TYPE, pw->type);
  }
  if (pw->password) {
    jsonAddStringValue(json, PW_KEY_PASSWORD, pw->password);
  }
  if (pw->expires_at) {
    jsonAddNumberValue(json, PW_KEY_EXPIRESAT, pw->expires_at);
  }
  if (pw->expires_after) {
    jsonAddNumberValue(json, PW_KEY_EXPIRESAFTER, pw->expires_after);
  }
  if (pw->command) {
    jsonAddStringValue(json, PW_KEY_COMMAND, pw->command);
  }
  return json;
}

char* passwordEntryToJSONString(const struct password_entry* pw) {
  if (pw == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* json = passwordEntryToJSON(pw);
  char*  ret  = jsonToString(json);
  secFreeJson(json);
  return ret;
}

struct password_entry* JSONStringToPasswordEntry(const char* json) {
  if (json == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  INIT_KEY_VALUE(PW_KEY_SHORTNAME, PW_KEY_TYPE, PW_KEY_PASSWORD,
                 PW_KEY_EXPIRESAT, PW_KEY_EXPIRESAFTER, PW_KEY_COMMAND);
  GET_JSON_VALUES_RETURN_NULL_ONERROR(json);
  KEY_VALUE_VARS(shortname, type, password, expires_at, expires_after, command);
  struct password_entry* pw = secAlloc(sizeof(struct password_entry));
  pwe_setShortname(pw, _shortname);
  pwe_setPassword(pw, _password);
  pwe_setCommand(pw, _command);
  pwe_setType(pw, strToUChar(_type));
  pwe_setExpiresAt(pw, strToULong(_expires_at));
  pwe_setExpiresAfter(pw, strToULong(_expires_after));
  secFree(_type);
  secFree(_expires_at);
  secFree(_expires_after);
  return pw;
}
