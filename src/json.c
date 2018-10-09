#include "json.h"

#include "utils/listUtils.h"
#include "utils/stringUtils.h"

#include <stdarg.h>
#include <syslog.h>

list_t* JSONArrayToList(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int         r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Token needed for parsing: %d",
         token_needed);
  if (token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return NULL;
  }
  jsmntok_t t[token_needed];
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t) / sizeof(t[0]));

  if (checkArrayParseResult(r, t[0]) != OIDC_SUCCESS) {
    return NULL;
  }
  int     j;
  list_t* l = list_new();
  l->free   = (void (*)(void*)) & clearFreeString;
  l->match  = (int (*)(void*, void*)) & strequal;
  for (j = 0; j < t[0].size; j++) {
    jsmntok_t* g = &t[j + 1];
    list_rpush(l, list_node_new(oidc_sprintf("%.*s", g->end - g->start,
                                             json + g->start)));
  }
  return l;
}

char* JSONArrrayToDelimitedString(const char* json, char delim) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int         r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Token needed for parsing: %d",
         token_needed);
  if (token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return NULL;
  }
  jsmntok_t t[token_needed];
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t) / sizeof(t[0]));

  if (checkArrayParseResult(r, t[0]) != OIDC_SUCCESS) {
    return NULL;
  }
  char* str = oidc_sprintf("");
  int   j;
  char* tmp = NULL;
  for (j = 0; j < t[0].size; j++) {
    jsmntok_t* g = &t[j + 1];
    tmp          = oidc_sprintf("%s%c%.*s", str, delim, g->end - g->start,
                       json + g->start);
    clearFreeString(str);
    if (tmp == NULL) {
      return NULL;
    }
    str = tmp;
  }
  return str;
}

/** @fn char* getJSONValue(const char* json, const char* key)
 * @brief parses a json string and returns the value for a given \p key
 * @param json the json string
 * @param key the key
 * @return the value for the \p key
 */
char* getJSONValue(const char* json, const char* key) {
  if (NULL == json || NULL == key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int         r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Token needed for parsing: %d",
         token_needed);
  if (token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return NULL;
  }
  jsmntok_t t[token_needed];
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t) / sizeof(t[0]));

  if (checkParseResult(r, t[0]) != OIDC_SUCCESS) {
    return NULL;
  }
  char* value = NULL;
  if ((value = getValuefromTokens(t, r, key, json)) == NULL) {
    oidc_errno = OIDC_EJSONNOFOUND;
    return NULL;
  }
  return value;
}

/** @fn int getJSONValues(const char* json, struct key_value* pairs, size_t
 * size)
 * @brief gets multiple values from a json string
 * @param json the json string to be parsed
 * @param pairs an array of key_value pairs. The keys are used as keys. A
 * pointer to the result is stored in the value field. The previous pointer is
 * not freed, thus it should be NULL.
 * @param size the number of key value pairs
 * return the number of set values or -1 on failure
 */
oidc_error_t getJSONValues(const char* json, struct key_value* pairs,
                           size_t size) {
  oidc_error_t e;
  if (NULL == json || NULL == pairs || size == 0) {
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
  unsigned int i;
  for (i = 0; i < size; i++) {
    pairs[i].value = getValuefromTokens(t, r, pairs[i].key, json);
  }
  return i;
}

int isJSONObject(const char* json) {
  if (json == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
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

  if (checkParseResult(r, t[0]) != OIDC_SUCCESS) {
    return 0;
  }
  return 1;
}

char* getValuefromTokens(jsmntok_t t[], int r, const char* key,
                         const char* json) {
  /* Loop over all keys of the root object */
  int i;
  for (i = 1; i < r; i++) {
    if (jsoneq(json, &t[i], key) == 0) {
      if (i == r - 1) {
        return NULL;
      }
      /* We may use strndup() to fetch string value */
      char* value = oidc_sprintf("%.*s", t[i + 1].end - t[i + 1].start,
                                 json + t[i + 1].start);
      value       = strelimIfFollowed(value, '\\',
                                '/');  // needed for escaped slashes, which are
                                       // json comforn but not correctly parsed
      value = strelimIfFollowed(value, '\\', '"');
      return value;
    }
  }
  return NULL;
}

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
  keys->free   = (void (*)(void*)) & clearFreeString;
  keys->match  = (int (*)(void*, void*)) & strequal;
  /* Loop over all keys of the root object */
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
    clearFreeString(value);
  }
  return keys;
}

int jsoneq(const char* json, jsmntok_t* tok, const char* s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

oidc_error_t checkParseResult(int r, jsmntok_t t) {
  if (r < 0) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Failed to parse JSON: %d\n", r);
    oidc_errno = OIDC_EJSONPARS;
    return OIDC_EJSONPARS;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t.type != JSMN_OBJECT) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Object expected\n");
    oidc_errno = OIDC_EJSONOBJ;
    return OIDC_EJSONOBJ;
  }
  return OIDC_SUCCESS;
}

oidc_error_t checkArrayParseResult(int r, jsmntok_t t) {
  if (r < 0) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Failed to parse JSON: %d\n", r);
    oidc_errno = OIDC_EJSONPARS;
    return OIDC_EJSONPARS;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t.type != JSMN_ARRAY) {
    syslog(LOG_AUTHPRIV | LOG_ERR, "Array expected\n");
    oidc_errno = OIDC_EJSONARR;
    return oidc_errno;
  }
  return OIDC_SUCCESS;
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
  clearFreeString(json);
  oidc_errno = OIDC_SUCCESS;
  return tmp;
}

char* json_addValue(char* json, const char* key, const char* value) {
  if (key == NULL || value == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (json == NULL) {
    json = oidc_strcopy("{}");
  }
  const char* const fmt = "%s, \"%s\":%s}";
  int               len = strlen(json);
  if (json[len - 1] != '}') {
    oidc_errno = OIDC_EJSONADD;
    return json;
  }
  json[len - 1] = '\0';
  char* tmp     = oidc_sprintf(fmt, json, key, value);
  if (tmp == NULL) {
    return json;
  }
  if (tmp[1] == ',') {
    tmp[1] = ' ';
  }
  clearFree(json, len);
  oidc_errno = OIDC_SUCCESS;
  return tmp;
}

char* json_addStringValue(char* json, const char* key, const char* value) {
  if (key == NULL || value == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (json == NULL) {
    json = oidc_strcopy("{}");
  }
  char* tmp = oidc_sprintf("\"%s\"", value);
  if (tmp == NULL) {
    return json;
  }
  char* res = json_addValue(json, key, tmp);
  clearFreeString(tmp);
  return res;
}

int json_hasKey(char* json, const char* key) {
  char* value = getJSONValue(json, key);
  if (strValid(value)) {
    clearFreeString(value);
    return 1;
  } else {
    return 0;
  }
}

/**
 * last argument has to be NULL
 * Only use pairs of 3 (char*, char*, int)
 */
char* generateJSONObject(char* k1, char* v1, int isString1, ...) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generating JSONObject");
  va_list args;
  va_start(args, isString1);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "First key:value is %s:%s", k1, v1);
  char* json =
      oidc_sprintf(isString1 ? "{\"%s\":\"%s\"}" : "{\"%s\":%s}", k1, v1);
  char* key;
  while ((key = va_arg(args, char*)) != NULL) {
    char* value    = va_arg(args, char*);
    int   isString = va_arg(args, int);
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "key:value is %s:%s", key, value);
    json = isString ? json_addStringValue(json, key, value)
                    : json_addValue(json, key, value);
    if (json == NULL) {
      return NULL;
    }
  }
  return json;
}
/**
 * last argument has to be NULL
 */
char* generateJSONArray(char* v1, ...) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Generating JSONArray");
  va_list args;
  va_start(args, v1);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "First value is %s", v1);
  char* array = oidc_sprintf("[\"%s\"]", v1);
  char* v;
  while ((v = va_arg(args, char*)) != NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "value is %s", v);
    array = json_arrAdd(array, v);
  }
  return array;
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

char* mergeJSONObject(const char* j1, const char* j2) {
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
      clearFreeString(val1);
      clearFreeString(val2);
      return NULL;
    }
    clearFreeString(val1);
    clearFreeString(val2);
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
    clearFreeString(val);
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
    clearFreeString(val);
  }
  list_iterator_destroy(it);
  list_destroy(j2_unique_keys);
  return json;
}
