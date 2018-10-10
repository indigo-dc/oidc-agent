#include "json.h"

#include "oidc_error.h"
#include "utils/listUtils.h"
#include "utils/stringUtils.h"

#include <stdarg.h>
#include <syslog.h>

cJSON_Hooks hooks;
int         jsonInitDone = 0;

void initCJSON() {  // TODO
  if (!jsonInitDone) {
    hooks.malloc_fn = secAlloc;
    hooks.free_fn   = secFree;
    cJSON_InitHooks(&hooks);
    jsonInitDone = 1;
  }
}
char* jsonToString(cJSON* cjson) {
  if (cjson == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  return cJSON_Print(cjson);
}

cJSON* stringToJson(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Parsing json '%s'", json);
  cJSON* cj = cJSON_Parse(json);
  if (cj == NULL) {
    oidc_errno = OIDC_EJSONPARS;
  }
  return cj;
}

int isJSONObject(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return 0;
  }
  return cJSON_IsObject(cj);
}

void secFreeJson(cJSON* cjson) {
  if (cjson == NULL) {
    return;
  }
  cJSON_Delete(cjson);
}

int jsonStringHasKey(const char* json, const char* key) {
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return 0;
  }
  int res = jsonHasKey(cj, key);
  secFreeJson(cj);
  return res;
}

int jsonHasKey(const cJSON* cjson, const char* key) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  char* value = getJSONValue(cjson, key);
  if (strValid(value)) {
    secFree(value);
    return 1;
  } else {
    return 0;
  }
}

char* getJSONItemValue(cJSON* valueItem) {
  if (NULL == valueItem) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (cJSON_IsString(valueItem)) {
    return oidc_strcopy(cJSON_GetStringValue(valueItem));
  }
  return cJSON_Print(valueItem);
}

char* getJSONValue(const cJSON* cjson, const char* key) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (!cJSON_HasObjectItem(cjson, key)) {
    oidc_errno = OIDC_EJSONNOFOUND;
    return NULL;
  }
  cJSON* valueItem = cJSON_GetObjectItemCaseSensitive(cjson, key);
  return getJSONItemValue(valueItem);
}

char* getJSONValueFromString(const char* json, const char* key) {
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  char* value = getJSONValue(cj, key);
  secFreeJson(cj);
  return value;
}

oidc_error_t getJSONValues(const cJSON* cjson, struct key_value* pairs,
                           size_t size) {
  if (NULL == cjson || NULL == pairs || size == 0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  unsigned int i;
  for (i = 0; i < size; i++) {
    pairs[i].value = getJSONValue(cjson, pairs[i].key);
  }
  return i;
}

oidc_error_t getJSONValuesFromString(const char* json, struct key_value* pairs,
                                     size_t size) {
  if (NULL == json || NULL == pairs || size == 0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return oidc_errno;
  }
  oidc_error_t e = getJSONValues(cj, pairs, size);
  secFreeJson(cj);
  return e;
}

list_t* JSONArrayToList(const cJSON* cjson) {
  if (NULL == cjson) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (!cJSON_IsArray(cjson)) {
    oidc_errno = OIDC_EJSONARR;
    return NULL;
  }

  int     j;
  list_t* l = list_new();
  l->free   = secFree;
  l->match  = (int (*)(void*, void*)) & strequal;
  for (j = 0; j < cJSON_GetArraySize(cjson); j++) {
    list_rpush(l, list_node_new(oidc_strcopy(
                      getJSONItemValue(cJSON_GetArrayItem(cjson, j)))));
  }
  return l;
}

list_t* JSONArrayStringToList(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  list_t* l = JSONArrayToList(cj);
  secFreeJson(cj);
  return l;
}

char* JSONArrayToDelimitedString(const cJSON* cjson, char delim) {
  if (NULL == cjson) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* list = JSONArrayToList(cjson);
  char*   str  = listToDelimitedString(list, delim);
  list_destroy(list);
  return str;
}

char* JSONArrayStringToDelimitedString(const char* json, char delim) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* cj = stringToJson(json);
  if (cj == NULL) {
    return NULL;
  }
  char* str = JSONArrayToDelimitedString(cj, delim);
  secFreeJson(cj);
  return str;
}

cJSON* jsonAddStringValue(cJSON* cjson, const char* key, const char* value) {
  if (NULL == cjson || NULL == key || NULL == value) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON_AddStringToObject(cjson, key, value);
  return cjson;
}
cJSON* jsonAddNumberValue(cJSON* cjson, const char* key, const double value) {
  if (NULL == cjson || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON_AddNumberToObject(cjson, key, value);
  return cjson;
}
cJSON* jsonAddArrayValue(cJSON* cjson, const char* key,
                         const char* json_array) {
  if (NULL == cjson || NULL == key || NULL == json_array) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* array = stringToJson(json_array);
  cJSON_AddItemToObject(cjson, key, array);
  return cjson;
}
cJSON* jsonAddObjectValue(cJSON* cjson, const char* key,
                          const char* json_object) {
  if (NULL == cjson || NULL == key || NULL == json_object) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* object = stringToJson(json_object);
  cJSON_AddItemToObject(cjson, key, object);
  return cjson;
}
cJSON* jsonAddJSON(cJSON* cjson, const char* key, cJSON* item) {
  if (NULL == cjson || NULL == key || NULL == item) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON_AddItemToObject(cjson, key, item);
  return cjson;
}

cJSON* listToJSONArray(list_t* list) {
  if (list == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  cJSON* json = cJSON_CreateArray();
  if (json == NULL) {
    oidc_seterror("Coud not create json array");
    return NULL;
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(list, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    cJSON_AddItemToArray(json, cJSON_CreateString(node->val));
  }
  list_iterator_destroy(it);
  return json;
}

/**
 * last argument has to be NULL
 * Only use pairs of 3 (char*, char*, int)
 */
cJSON* generateJSONObject(char* k1, char* v1, int type1, ...) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generating JSONObject");
  va_list args;
  va_start(args, type1);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "First key:value is %s:%s", k1, v1);
  cJSON* json = cJSON_CreateObject();
  if (json == NULL) {
    oidc_seterror("Coud not create json object");
    return NULL;
  }
  char* key   = k1;
  char* value = v1;
  int   type  = type1;
  do {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "key:value is %s:%s", key, value);
    cJSON* (*addFunc)(cJSON*, const char*, const char*);
    switch (type) {
      case cJSON_String: addFunc = jsonAddStringValue; break;
      case cJSON_Object: addFunc = jsonAddObjectValue; break;
      case cJSON_Array: addFunc = jsonAddArrayValue; break;
      case cJSON_Number:
        syslog(LOG_AUTHPRIV | LOG_ERR,
               "generating JSONObjects with numbers not supported");
        oidc_errno = OIDC_NOTIMPL;
        return NULL;
      default:
        syslog(LOG_AUTHPRIV | LOG_ERR, "unknown type %d", type);
        oidc_errno = OIDC_EJSONTYPE;
        return NULL;
    }
    json = addFunc(json, key, value);
    if (json == NULL) {
      return NULL;
    }

    key   = va_arg(args, char*);
    value = va_arg(args, char*);
    type  = va_arg(args, int);
  } while (key != NULL);
  return json;
}

/**
 * last argument has to be NULL
 */
cJSON* generateJSONArray(char* v1, ...) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generating JSONArray");
  va_list args;
  va_start(args, v1);
  cJSON* json = cJSON_CreateArray();
  if (json == NULL) {
    oidc_seterror("Coud not create json array");
    return NULL;
  }
  char* v = v1;
  while (v != NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "value is %s", v);
    cJSON_AddItemToArray(json, cJSON_CreateString(v));
    v = va_arg(args, char*);
  }
  return json;
}
/*


list_t* getJSONKeys(const char* json, int strHasToBeValid) {
  oidc_error_t e;
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Parsing json '%s'", json);
  int         r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  if (token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Token needed for parsing: %d",
         token_needed);
  jsmntok_t t[token_needed];
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t) / sizeof(t[0]));

  if ((e = checkParseResult(r, t[0])) != OIDC_SUCCESS) {
    return NULL;
  }
  return getKeysfromTokens(t, r, json, strHasToBeValid);
}

list_t* getKeysfromTokens(jsmntok_t t[], int r, const char* json,
                          int strHasToBeValid) {
  list_t* keys = list_new();
  keys->free   = (void (*)(void*)) & secFree;
  keys->match  = (int (*)(void*, void*)) & strequal;
  /* Loop over all keys of the root object * /
  int i = 1;
  while (i < r - 1) {
    char* value = oidc_sprintf("%.*s", t[i + 1].end - t[i + 1].start,
                               json + t[i + 1].start);
    switch (t[i + 1].type) {
      case JSMN_STRING:
      case JSMN_PRIMITIVE:
        if (!strHasToBeValid || strValid(value)) {
          list_rpush(keys,
                     list_node_new(oidc_sprintf("%.*s", t[i].end - t[i].start,
                                                json + t[i].start)));
        }
        i += 2;
        break;
      case JSMN_ARRAY:
        if (!strHasToBeValid || (strValid(value) && !strequal(value, "[]"))) {
          list_rpush(keys,
                     list_node_new(oidc_sprintf("%.*s", t[i].end - t[i].start,
                                                json + t[i].start)));
        }
        i += t[i + 1].size + 2;
        break;
      case JSMN_OBJECT:
        if (!strHasToBeValid || (strValid(value) && !strequal(value, "{}"))) {
          list_rpush(keys,
                     list_node_new(oidc_sprintf("%.*s", t[i].end - t[i].start,
                                                json + t[i].start)));
        }
        i += t[i + 1].size * 2 + 2;
        break;
      case JSMN_UNDEFINED: list_destroy(keys); return NULL;
    }
    secFree(value);
  }
  return keys;
}

char* json_arrAdd(char* json, const char* value) {
  if (json == NULL || value == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  const char* const fmt = "%s, \"%s\"]";
  int               len = strlen(json);
  if (json[len - 1] != ']') {
    oidc_errno = OIDC_EJSONADD;
    return json;
  }
  json[len - 1] = '\0';
  char* tmp     = oidc_sprintf(fmt, json, value);
  if (tmp == NULL) {
    return json;
  }
  if (tmp[1] == ',') {
    memmove(tmp + 1, tmp + 3,
            strlen(tmp + 3));  // removes the the added comma and space if there
                               // was no element in the array
    tmp[strlen(tmp) - 1] = '\0';
    tmp[strlen(tmp) - 1] = '\0';  // removes the two last char (we moved
                                  // everything two chars to the front)
  }
  secFree(json);
  oidc_errno = OIDC_SUCCESS;
  return tmp;
}




jsmntype_t getValueTypeForKey(const char* json, const char* key) {
  oidc_error_t e;
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Parsing json '%s'", json);
  int         r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  if (token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return OIDC_EJSONPARS;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Token needed for parsing: %d",
         token_needed);
  jsmntok_t t[token_needed];
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t) / sizeof(t[0]));

  if ((e = checkParseResult(r, t[0])) != OIDC_SUCCESS) {
    return e;
  }
  int i;
  for (i = 1; i < r - 1; i++) {
    if (jsoneq(json, &t[i], key) == 0) {
      return t[i + 1].type;
    }
  }

  return JSMN_UNDEFINED;
}

*/

// TODO
cJSON* mergeJSONObject(const cJSON* j1, const cJSON* j2) {
  if (j1 == NULL || j2 == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* j1_keys         = getJSONKeys(j1, 1);
  list_t* j2_keys         = getJSONKeys(j2, 1);
  list_t* duplicated_keys = intersectLists(j1_keys, j2_keys);
  list_t* j2_unique_keys  = subtractLists(j2_keys, duplicated_keys);
  list_destroy(j2_keys);
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(duplicated_keys, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* key  = node->val;
    char* val1 = getJSONValue(j1, key);
    char* val2 = getJSONValue(j2, key);
    if (getValueTypeForKey(j1, key) == JSMN_ARRAY) {
      val1 = strelimIfAfter(val1, ' ', ',');
      val2 = strelimIfAfter(val2, ' ', ',');
    }

    if (!strequal(val1, val2)) {
      oidc_errno = OIDC_EJSONMERGE;
      syslog(LOG_AUTHPRIV | LOG_ERR,
             "Cannot merge json objects: Conflict for key '%s' between "
             "value '%s' and '%s'",
             key, val1, val2);
      list_iterator_destroy(it);
      list_destroy(duplicated_keys);
      list_destroy(j1_keys);
      list_destroy(j2_unique_keys);
      secFree(val1);
      secFree(val2);
      return NULL;
    }
    secFree(val1);
    secFree(val2);
  }
  list_iterator_destroy(it);
  list_destroy(duplicated_keys);
  char* json = NULL;
  it         = list_iterator_new(j1_keys, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* key = node->val;
    char* val = getJSONValue(j1, key);
    char* (*add_func)(char*, const char*, const char*) =
        getValueTypeForKey(j1, key) == JSMN_STRING ? json_addStringValue
                                                   : json_addValue;
    json = add_func(json, key, val);
    secFree(val);
  }
  list_iterator_destroy(it);
  list_destroy(j1_keys);
  it = list_iterator_new(j2_unique_keys, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* key = node->val;
    char* val = getJSONValue(j2, key);
    char* (*add_func)(char*, const char*, const char*) =
        getValueTypeForKey(j2, key) == JSMN_STRING ? json_addStringValue
                                                   : json_addValue;
    json = add_func(json, key, val);
    secFree(val);
  }
  list_iterator_destroy(it);
  list_destroy(j2_unique_keys);
  return json;
}
