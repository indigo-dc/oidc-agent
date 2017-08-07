#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "json.h"



/** @fn char* getJSONValue(const char* json, const char* key)
 * @brief parses a json string and returns the value for a given \p key
 * @param json the json string
 * @param key the key
 * @return the value for the \p key
 */
char* getJSONValue(const char* json, const char* key) {
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(checkParseResult(r, t[0]))	
    return getValuefromTokens(t, r, key, json);
  else
    return NULL;
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
int getJSONValues(const char* json, struct key_value* pairs, size_t size) {
  int r;
  jsmn_parser p;
  jsmn_init(&p);
  int token_needed = jsmn_parse(&p, json, strlen(json), NULL, 0);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Token needed for parsing: %d",token_needed);
  jsmntok_t t[token_needed]; 
  jsmn_init(&p);
  r = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(!checkParseResult(r, t[0]))
    return -1;
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
      /* We may use strndup() to fetch string value */
      char* value = calloc(sizeof(char),t[i+1].end-t[i+1].start+1);
      sprintf(value,"%.*s", t[i+1].end-t[i+1].start,
          json + t[i+1].start);
      return value;
    } 
  }
  return NULL;
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

int checkParseResult(int r, jsmntok_t t) {
  if (r < 0) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Failed to parse JSON: %d\n", r);
    return 0;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t.type != JSMN_OBJECT) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "Object expected\n");
    return 0;
  }
  return 1;
}


