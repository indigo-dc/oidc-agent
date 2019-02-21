#ifndef KEY_VALUE_H
#define KEY_VALUE_H

#include "memory.h"

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


#endif  // KEY_VALUE_H
