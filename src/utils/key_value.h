#ifndef KEY_VALUE_H
#define KEY_VALUE_H

#include "utils/macros.h"
#include "utils/memory.h"

#include <stddef.h>

struct key_value {
  const char* key;
  char*       value;
};

static inline void secFreeKeyValuePairs(struct key_value* pairs, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) { _secFree(pairs[i].value); }
}

#define KEY_VALUE(i, keyname)   \
  pairs[(i)].key   = (keyname); \
  pairs[(i)].value = NULL;

#define KEY_VALUE_VAR(i, valuename) char* _##valuename = pairs[(i)].value;

#define KEY_VALUE_VARS(...) \
  CALL_MACRO_X_FOR_EACH_WITH_N(KEY_VALUE_VAR, __VA_ARGS__)

#define INIT_KEY_VALUE(...)                           \
  struct key_value pairs[COUNT_VARARGS(__VA_ARGS__)]; \
  CALL_MACRO_X_FOR_EACH_WITH_N(KEY_VALUE, __VA_ARGS__)

#define GET_JSON_VALUES_RETURN_X_ONERROR(json, returnvalue)                    \
  if (getJSONValuesFromString((json), pairs, sizeof(pairs) / sizeof(*pairs)) < \
      0) {                                                                     \
    secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs));               \
    return (returnvalue);                                                      \
  }

#define GET_JSON_VALUES_RETURN_NULL_ONERROR(json) \
  GET_JSON_VALUES_RETURN_X_ONERROR((json), NULL)

#define GET_JSON_VALUES_RETURN_OIDCERRNO_ONERROR(json) \
  GET_JSON_VALUES_RETURN_X_ONERROR((json), oidc_errno)

#define SEC_FREE_KEY_VALUES() \
  secFreeKeyValuePairs(pairs, sizeof(pairs) / sizeof(*pairs))

#define CALL_GETJSONVALUES(json) \
  getJSONValuesFromString((json), pairs, sizeof(pairs) / sizeof(*pairs))

#define CALL_GETJSONVALUES_FROM_CJSON(json) \
  getJSONValues((json), pairs, sizeof(pairs) / sizeof(*pairs))

#define RESET_KEY_VALUE_VALUES_TO_NULL()                       \
  for (size_t _i; _i < sizeof(pairs) / sizeof(*pairs); _i++) { \
    pairs[_i].value = NULL;                                    \
  }

#endif  // KEY_VALUE_H
