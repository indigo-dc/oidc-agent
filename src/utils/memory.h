#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void* secAlloc(size_t size);
void* secCalloc(size_t nmemb, size_t size);
void* secRealloc(void* p, size_t size);
void  _secFree(void* p);
void  _secFreeN(void* p, size_t len);
void  _secFreeArray(char** arr, size_t size);

#ifndef secFree
#define secFree(ptr) \
  do {               \
    _secFree((ptr)); \
    (ptr) = NULL;    \
  } while (0)
#endif  // secFree
#define secFreeN(ptr, len)   \
  do {                       \
    _secFreeN((ptr), (len)); \
    (ptr) = NULL;            \
  } while (0)
#define secFreeArray(ptr, size)   \
  do {                            \
    _secFreeArray((ptr), (size)); \
    (ptr) = NULL;                 \
  } while (0)

#endif  // MEMORY_H
