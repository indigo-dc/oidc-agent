#include "password_entry.h"
#include "utils/json.h"

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
  size_t           len = 5;
  struct key_value pairs[len];
  for (size_t i = 0; i < len; i++) { pairs[i].value = NULL; }
  pairs[0].key = PW_KEY_SHORTNAME;
  pairs[1].key = PW_KEY_TYPE;
  pairs[2].key = PW_KEY_PASSWORD;
  pairs[3].key = PW_KEY_EXPIRESAT;
  pairs[4].key = PW_KEY_COMMAND;
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) <
      0) {
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));
    return NULL;
  }
  struct password_entry* pw = secAlloc(sizeof(struct password_entry));
  pwe_setShortname(pw, pairs[0].value);
  pwe_setPassword(pw, pairs[2].value);
  pwe_setCommand(pw, pairs[4].value);
  pwe_setType(pw, strToUChar(pairs[1].value));
  pwe_setExpiresAt(pw, strToULong(pairs[3].value));
  secFree(pairs[1].value);
  secFree(pairs[3].value);
  return pw;
}
