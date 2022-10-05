#include "mytokenUtils.h"

#include <string.h>

#include "defines/msys.h"
#include "utils/duration.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"
#include "wrapper/cjson.h"

const char* _mytoken_user_base = NULL;

#ifndef ANY_MSYS
#define _MYTOKEN_USER_BASE_CONF "~/.config/mytoken/"
#define _MYTOKEN_USER_BASE_DOT "~/.mytoken/"
#define _MYTOKEN_GLOBAL_BASE "/etc/mytoken/"
#else
#define AGENTDIR_LOCATION_CONFIG "$LOCALAPPDATA/mytoken/"
#define AGENTDIR_LOCATION_DOT "$USERPROFILE/Documents/mytoken/"
// global base dir is defined in settings.c
#endif

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
  char* globalP = oidc_pathcat(
#ifdef ANY_MSYS
      MYTOKEN_GLOBAL_BASE(),
#else
      _MYTOKEN_GLOBAL_BASE,
#endif
      relPath);
  char* _global = readFile(globalP);
  secFree(globalP);
  cJSON* global = stringToJsonDontLogError(_global);
  if (_global != NULL && global == NULL) {
    if (lastChar(_global) == '\n') {
      lastChar(_global) = '\0';
    }
    char* quoted = oidc_sprintf("\"%s\"", _global);
    global       = stringToJsonDontLogError(quoted);
    secFree(quoted);
  }
  secFree(_global);
  const char* userBasePath = getMytokenUserBasePath();
  if (!strValid(userBasePath)) {
    return global;
  }
  char* userP = oidc_pathcat(userBasePath, relPath);
  char* _user = readFile(userP);
  secFree(userP);
  cJSON* user = stringToJsonDontLogError(_user);
  if (_user != NULL && user == NULL) {
    if (lastChar(_user) == '\n') {
      lastChar(_user) = '\0';
    }
    char* quoted = oidc_sprintf("\"%s\"", _user);
    user         = stringToJsonDontLogError(quoted);
    secFree(quoted);
  }
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
  if (*name != '@') {
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

cJSON* _parseIncludes(cJSON* pre_final, const cJSON* includes,
                      cJSON* (*readFnc)(const char*)) {
  if (includes == NULL ||
      (!cJSON_IsString(includes) && !cJSON_IsArray(includes))) {
    return pre_final;
  }
  if (cJSON_IsString(includes)) {
    cJSON* incl = createFinalTemplate(includes, readFnc);
    _includeJSON(&pre_final, incl, readFnc);
    secFreeJson(incl);
    return pre_final;
  }
  cJSON* include = includes->child;
  while (include) {
    cJSON* incl = createFinalTemplate(include, readFnc);
    _includeJSON(&pre_final, incl, readFnc);
    secFreeJson(incl);
    include = include->next;
  }
  return pre_final;
}

cJSON* _createFinalTemplate(const cJSON* content,
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
    //  must be one or multiple template names
    char* name = cJSON_GetStringValue(content);
    if (strCountChar(name, ' ') == 0) {
      // must be a single template name
      normalizeTemplateName(name);
      cJSON* new_content = readFnc(name);
      cJSON* final       = createFinalTemplate(new_content, readFnc);
      secFreeJson(new_content);
      return final;
    }
    char* includes_str =
        delimitedStringToJSONArray(cJSON_GetStringValue(content), ' ');
    cJSON* includes = cJSON_Parse(includes_str);
    secFree(includes_str);
    cJSON* final = _parseIncludes(cJSON_Parse("{}"), includes, readFnc);
    secFreeJson(includes);
    return final;
  }

  cJSON* final = cJSON_Duplicate(content, 1);
  if (cJSON_IsObject(final)) {
    cJSON_DeleteItemFromObjectCaseSensitive(final, "include");
  }
  cJSON* includes = cJSON_GetObjectItemCaseSensitive(content, "include");
  final           = _parseIncludes(final, includes, readFnc);
  return final;
}

cJSON* createFinalTemplate(const cJSON* content,
                           cJSON* (*readFnc)(const char*)) {
  cJSON* final = _createFinalTemplate(content, readFnc);
  if (cJSON_IsObject(final)) {
    cJSON_DeleteItemFromObjectCaseSensitive(final, "include");
  }
  return final;
}

cJSON* parseRotationTemplate(const cJSON* content) {
  return createFinalTemplate(content, readRotationTemplate);
}
cJSON* parseRestrictionsTemplate(const cJSON* content) {
  if (cJSON_IsString(content)) {
    cJSON* t = readRestrictionsTemplate(
        normalizeTemplateName(cJSON_GetStringValue(content)));
    cJSON* f = parseRestrictionsTemplate(t);
    secFreeJson(t);
    return f;
  }
  cJSON* asArray = NULL;
  if (cJSON_IsObject(content)) {
    asArray = cJSON_CreateArray();
    cJSON_AddItemToArray(asArray, cJSON_Duplicate(content, 1));
  }
  cJSON* f = createFinalTemplate(asArray ?: content, readRestrictionsTemplate);
  secFreeJson(asArray);
  for (int i = cJSON_GetArraySize(f) - 1; i >= 0; i--) {
    cJSON* ai = cJSON_GetArrayItem(f, i);
    if (cJSON_GetArraySize(ai) == 0) {
      // Removing empty clauses
      cJSON_DeleteItemFromArray(f, i);
    } else {
      cJSON* nbf = cJSON_DetachItemFromObjectCaseSensitive(ai, "nbf");
      cJSON* exp = cJSON_DetachItemFromObjectCaseSensitive(ai, "exp");
      if (nbf) {
        time_t nbf_t = parseTime(cJSON_GetStringValue(nbf));
        cJSON_AddNumberToObject(ai, "nbf", (double)nbf_t);
      }
      if (exp) {
        time_t exp_t = parseTime(cJSON_GetStringValue(exp));
        cJSON_AddNumberToObject(ai, "exp", (double)exp_t);
      }
      secFreeJson(nbf);
      secFreeJson(exp);
    }
  }
  return f;
}
cJSON* parseCapabilityTemplate(const cJSON* content) {
  list_t* capabilities = NULL;
  if (cJSON_IsArray(content)) {
    capabilities = JSONArrayToList(content);
  } else if (cJSON_IsString(content)) {
    capabilities = delimitedStringToList(cJSON_GetStringValue(content), ' ');
  } else {
    return NULL;
  }
  cJSON*           final_caps = cJSON_CreateArray();
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(capabilities, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* elem = node->val;
    if (elem == NULL) {
      continue;
    }
    if (elem[0] != '@') {
      jsonArrayAddStringValue(final_caps, elem);
    } else {
      cJSON* included_json =
          readCapabilityTemplate(normalizeTemplateName(elem));
      cJSON* included_json_parsed = parseCapabilityTemplate(included_json);
      secFreeJson(included_json);
      appendArrayToArray(final_caps, included_json_parsed);
      secFreeJson(included_json_parsed);
    }
  }
  list_iterator_destroy(it);
  secFreeList(capabilities);
  return uniquifyArray(final_caps);
}
void _addTemplateToProfile(cJSON* profile, const char* key,
                           cJSON* (*templateParser)(const cJSON*)) {
  cJSON* t = cJSON_DetachItemFromObject(profile, key);
  cJSON_AddItemToObject(profile, key, templateParser(t));
  secFreeJson(t);
}
cJSON* parseProfile(const cJSON* content) {
  cJSON* j = createFinalTemplate(content, readProfile);
  _addTemplateToProfile(j, "rotation", parseRotationTemplate);
  _addTemplateToProfile(j, "restrictions", parseRestrictionsTemplate);
  _addTemplateToProfile(j, "capabilities", parseCapabilityTemplate);
  return j;
}
