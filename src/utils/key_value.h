#ifndef KEY_VALUE_H
#define KEY_VALUE_H

#include "utils/memory.h"
#include "utils/macros.h"

#include <stddef.h>

struct key_value {
  const char* key;
  char*       value;
};

static inline void secFreeKeyValuePairs(struct key_value* pairs, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) { secFree(pairs[i].value); }
}

#define KEY_VALUE(i, keyname) \
  pairs[(i)].key   = (keyname);          \
  pairs[(i)].value = NULL;               \

#define KEY_VALUE_VAR(i, valuename) \
  char* _##valuename = pairs[(i)].value;

#define KEY_VALUE_VARS(...) CALL_MACRO_X_FOR_EACH_WITH_N(KEY_VALUE_VAR, __VA_ARGS__)
#define INIT_KEY_VALUE(...) struct key_value pairs[COUNT_VARARGS(__VA_ARGS__)]; CALL_MACRO_X_FOR_EACH_WITH_N(KEY_VALUE, __VA_ARGS__)


#endif  // KEY_VALUE_H
