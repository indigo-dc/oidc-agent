#include "keySet.h"

#include "utils/memory.h"

void _secFreeKeySetSEstr(struct keySetSEstr keys) {
  if (keys.sign != keys.enc) {
    secFree(keys.enc);
  }
  secFree(keys.sign);
}
