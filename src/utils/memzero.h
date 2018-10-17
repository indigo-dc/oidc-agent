#ifndef MEMZERO_H
#define MEMZERO_H

#include <stddef.h>
#include <string.h>

static void* (*const volatile memset_ptr)(void*, int, size_t) = memset;

static inline void moresecure_memzero(void* p, size_t len) {
  (memset_ptr)(p, 0, len);
}

#endif  // MEMZERO_H
