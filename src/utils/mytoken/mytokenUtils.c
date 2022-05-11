#include "mytokenUtils.h"

#include <string.h>

#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"
#include "wrapper/cjson.h"

const char* _mytoken_user_base = NULL;

#define _MYTOKEN_USER_BASE_CONF "~/.config/mytoken/"
#define _MYTOKEN_USER_BASE_DOT "~/.mytoken/"
#define _MYTOKEN_GLOBAL_BASE "/etc/mytoken/"

const char* getMytokenUserBasePath() {
  if (_mytoken_user_base == NULL) {
    char* _mytoken_user_base_conf = fillEnvVarsInPath(_MYTOKEN_USER_BASE_CONF);
    char* _mytoken_user_base_dot  = fillEnvVarsInPath(_MYTOKEN_USER_BASE_DOT);
    if (_mytoken_user_base_conf == NULL && _mytoken_user_base_dot == NULL) {
      return NULL;
    }
    list_t* _mytoken_user_bases =
        createList(0, _mytoken_user_base_conf, _mytoken_user_base_dot, NULL);
    _mytoken_user_bases->free = _secFree;
    _mytoken_user_base        = getExistingLocation(_mytoken_user_bases);
    secFreeList(_mytoken_user_bases);
  }
  return _mytoken_user_base;
}

cJSON* readMytokenFile(const char* relPath) {
  char* globalP = oidc_pathcat(_MYTOKEN_GLOBAL_BASE, relPath);
  char* _global = readFile(globalP);
  secFree(globalP);
  cJSON* global = stringToJsonDontLogError(_global);
  secFree(_global);
  char* userP = oidc_pathcat(getMytokenUserBasePath(), relPath);
  char* _user = readFile(userP);
  secFree(userP);
  cJSON* user = stringToJsonDontLogError(_user);
  secFree(_user);
  if (user == NULL) {
    return global;
  }
  if (global == NULL) {
    return user;
  }
  cJSON* combined = jsonMergePatch(global, user);
  secFreeJson(user);
  secFreeJson(global);
  return combined;
}

cJSON* _readTemplate(const char* templateDir, const char* name) {
  char*  relPath = oidc_pathcat(templateDir, name);
  cJSON* ret     = readMytokenFile(relPath);
  secFree(relPath);
  return ret;
}

cJSON* readRestrictionsTemplate(const char* name) {
  return _readTemplate("restrictions.d", name);
}
cJSON* readCapabilityTemplate(const char* name) {
  return _readTemplate("capabilities.d", name);
}
cJSON* readRotationTemplate(const char* name) {
  return _readTemplate("rotation.d", name);
}
cJSON* readProfile(const char* name) {
  return _readTemplate("profiles.d", name);
}

char* normalizeTemplateName(char* name) {
  if (name == NULL || strlen(name) == 0) {
    return name;
  }
  if (*name != '!') {
    return name;
  }
  memmove(name, name + 1, strlen(name + 1));
  lastChar(name) = '\0';
  return name;
}

void _includeJSON(cJSON** base, const cJSON* include,
                  cJSON* (*readFnc)(const char*)) {
  if (base == NULL || *base == NULL ||
      (!cJSON_IsArray(*base) && !cJSON_IsObject(*base))) {
    return;
  }
  cJSON* includeJSON = createFinalTemplate(include, readFnc);
  if (includeJSON == NULL) {
    return;
  }
  if (cJSON_IsObject(includeJSON) && cJSON_IsObject(*base)) {
    cJSON* tmp = jsonMergePatch(includeJSON, *base);
    secFreeJson(*base);
    secFreeJson(includeJSON);
    *base = tmp;
    return;
  }
  if (!cJSON_IsArray(*base)) {
    cJSON* tmp = cJSON_CreateArray();
    cJSON_AddItemToArray(tmp, *base);
    *base = tmp;
  }
  if (cJSON_IsObject(includeJSON)) {
    cJSON_AddItemToArray(*base, includeJSON);
  } else {
    appendArrayToArray(*base, includeJSON);
    secFreeJson(includeJSON);
  }
}

cJSON* createFinalTemplate(const cJSON* content,
                           cJSON* (*readFnc)(const char*)) {
  if (content == NULL) {
    return NULL;
  }
  unsigned char baseIsArray = cJSON_IsArray(content);
  if (baseIsArray) {
    cJSON*       final = cJSON_CreateArray();
    const cJSON* j     = content->child;
    while (j) {
      cJSON* jj = createFinalTemplate(j, readFnc);
      if (jj) {
        if (cJSON_IsArray(jj)) {
          appendArrayToArray(final, jj);
          secFreeJson(jj);
        } else {
          cJSON_AddItemToArray(final, jj);
        }
      }
      j = j->next;
    }
    return final;
  }
  if (cJSON_IsString(content)) {
    // must be a single template name
    char* name = cJSON_GetStringValue(content);
    normalizeTemplateName(name);
    cJSON* new_content = readFnc(name);
    cJSON* final       = createFinalTemplate(new_content, readFnc);
    secFreeJson(new_content);
    return final;
  }

  cJSON* final    = cJSON_Duplicate(content, 1);
  cJSON* includes = cJSON_GetObjectItemCaseSensitive(content, "include");
  if (includes == NULL ||
      (!cJSON_IsString(includes) && !cJSON_IsArray(includes))) {
    return final;
  }
  if (cJSON_IsString(includes)) {
    cJSON* incl = createFinalTemplate(includes, readFnc);
    _includeJSON(&final, incl, readFnc);
    secFreeJson(incl);
    return final;
  }
  cJSON* include = includes->child;
  while (include) {
    cJSON* incl = createFinalTemplate(include, readFnc);
    _includeJSON(&final, incl, readFnc);
    secFreeJson(incl);
    include = include->next;
  }
  return final;
}