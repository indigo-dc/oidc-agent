#include "json.h"

#include <syslog.h>

int JSONArrrayToArray(const char* json, char** arr) {
  if(NULL==json) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  if(token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return oidc_errno;
  }
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkArrayParseResult(r, t[0])!=OIDC_SUCCESS) {
    return oidc_errno;
  }
  if(arr==NULL) {
    return t[0].size;
  }
  int j;
  for (j = 0; j < t[0].size; j++) {
    jsmntok_t *g = &t[j+1];
    arr[j] = oidc_sprintf("%.*s", g->end - g->start, json + g->start);
  }
  return j;
}

list_t* JSONArrayToList(const char* json) {
  if(NULL==json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  if(token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return NULL;
  }
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkArrayParseResult(r, t[0])!=OIDC_SUCCESS) {
    return NULL;
  }
  int j;
  list_t* l = list_new();
  l->free = (void(*) (void*)) &clearFreeString;
  l->match = (int(*) (void*, void*)) &strequal;
  for (j = 0; j < t[0].size; j++) {
    jsmntok_t *g = &t[j+1];
    list_rpush(l, list_node_new(oidc_sprintf("%.*s", g->end - g->start, json + g->start)));
  }
  return l;

}

char* JSONArrrayToDelimitedString(const char* json, char delim) {
  if(NULL==json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  if(token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return NULL;
  }
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkArrayParseResult(r, t[0])!=OIDC_SUCCESS) {
    return NULL;
  }
  char* str = oidc_sprintf(""); 
  int j; char* tmp = NULL;
  for (j = 0; j < t[0].size; j++) {
    jsmntok_t *g = &t[j+1];
    tmp = oidc_sprintf("%s%c%.*s", str, delim, g->end - g->start, json + g->start);
    clearFreeString(str);
    if(tmp==NULL) {
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
  if(NULL==json || NULL==key) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  if(token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return NULL;
  }
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkParseResult(r, t[0])!=OIDC_SUCCESS)	{
    return NULL;
  }
  char* value = NULL;
  if((value = getValuefromTokens(t, r, key, json))==NULL) {
    oidc_errno = OIDC_EJSONNOFOUND;
    return NULL;
  }
  return value;
}

/** @fn int getJSONValues(const char* json, struct key_value* pairs, size_t size)
 * @brief gets multiple values from a json string
 * @param json the json string to be parsed
 * @param pairs an array of key_value pairs. The keys are used as keys. A
 * pointer to the result is stored in the value field. The previous pointer is
 * not freed, thus it should be NULL.
 * @param size the number of key value pairs
 * return the number of set values or -1 on failure
 */
oidc_error_t getJSONValues(const char* json, struct key_value* pairs, size_t size) {
  oidc_error_t e;
  if(NULL==json || NULL==pairs || size==0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  if(token_needed < 0) {
    oidc_errno = OIDC_EJSONPARS;
    return OIDC_EJSONPARS;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if((e = checkParseResult(r, t[0]))!=OIDC_SUCCESS) {
    return e;
  }
  unsigned int i;
  for(i=0; i<size;i++){
    pairs[i].value = getValuefromTokens(t, r, pairs[i].key, json);
  }
  return i;
}

char* getValuefromTokens(jsmntok_t t[], int r, const char* key, const char* json) {
  /* Loop over all keys of the root object */
  int i;
  for (i = 1; i < r; i++) {
    if (jsoneq(json, &t[i], key) == 0) {
      if(i==r-1) {
        return NULL;
      }
      /* We may use strndup() to fetch string value */
      char* value = oidc_sprintf("%.*s", t[i+1].end-t[i+1].start, json + t[i+1].start);
      value = strelimIfFollowed(value, '\\', '/'); // needed for escaped slashes, which are json comforn but not correctly parsed
      return value;
    } 
  }
  return NULL;
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if(tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

oidc_error_t checkParseResult(int r, jsmntok_t t) {
  if(r < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Failed to parse JSON: %d\n", r);
    oidc_errno = OIDC_EJSONPARS;
    return OIDC_EJSONPARS;
  }

  /* Assume the top-level element is an object */
  if(r < 1 || t.type != JSMN_OBJECT) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Object expected\n");
    oidc_errno = OIDC_EJSONOBJ;
    return OIDC_EJSONOBJ;
  }
  return OIDC_SUCCESS;
}

oidc_error_t checkArrayParseResult(int r, jsmntok_t t) {
  if(r < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Failed to parse JSON: %d\n", r);
    oidc_errno = OIDC_EJSONPARS;
    return OIDC_EJSONPARS;
  }

  /* Assume the top-level element is an object */
  if(r < 1 || t.type != JSMN_ARRAY) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Array expected\n");
    oidc_errno = OIDC_EJSONARR;
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}


char* json_arrAdd(char* json, const char* value) {
  if(json==NULL || value==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  const char* const fmt = "%s, \"%s\"]";
  int len = strlen(json);
  if(json[len-1]!=']') {
    oidc_errno = OIDC_EJSONADD;
    return json;
  }
  json[len-1] = '\0';
  char* tmp = oidc_sprintf(fmt, json, value);
  if(tmp==NULL) {
    return json;
  }
  if(tmp[1]==',') {
    memmove(tmp+1, tmp+3, strlen(tmp+3)); //removes the the added comma and space if there was no element in the array
    tmp[strlen(tmp)-1]='\0';
    tmp[strlen(tmp)-1]='\0'; //removes the two last char (we moved everything two chars to the front)
  }
  clearFreeString(json);
  oidc_errno = OIDC_SUCCESS;
  return tmp;

}

char* json_addValue(char* json, const char* key, const char* value) {
  if(json==NULL || key==NULL || value==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  const char* const fmt = "%s, \"%s\":%s}";
  int len = strlen(json);
  if(json[len-1]!='}') {
    oidc_errno = OIDC_EJSONADD;
    return json;
  }
  json[len-1] = '\0';
  char* tmp = oidc_sprintf(fmt, json, key, value);
  if(tmp==NULL) {
    return json;
  }
  if(tmp[1]==',') {
    tmp[1]=' ';
  }
  clearFreeString(json);
  oidc_errno = OIDC_SUCCESS;
  return tmp;
}

char* json_addStringValue(char* json, const char* key, char* value) {
  if(json==NULL || key==NULL || value==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* tmp = oidc_sprintf("\"%s\"", value);
  if(tmp==NULL) {
    return json;
  }
  char* res = json_addValue(json, key, tmp);
  clearFreeString(tmp);
  return res;
}

int json_hasKey(char* json, const char* key) {
  char* value = getJSONValue(json, key);
  if(isValid(value)) {
    clearFreeString(value);
    return 1;
  } else {
    return 0;
  }
}
