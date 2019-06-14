#ifndef MEMORY_H
#define MEMORY_H

#include "utils/macros.h"

#include <stddef.h>

void* secAlloc(size_t size);
void* secCalloc(size_t nmemb, size_t size);
void* secRealloc(void* p, size_t size);
void  _secFree(void* p);
void  _secFreeN(void* p, size_t len);
void  _secFreeArray(char** arr, size_t size);
void  _secFreeMultipleN(size_t n, void* p, ...);
void* oidc_memcopy(void* src, size_t size);

#ifndef secFree
#define secFree(ptr) \
  do {               \
    _secFree((ptr)); \
    (ptr) = NULL;    \
  } while (0)
#endif  // secFree
#ifndef secFreeN
#define secFreeN(ptr, len)   \
  do {                       \
    _secFreeN((ptr), (len)); \
    (ptr) = NULL;            \
  } while (0)
#endif  // secFreeN
#ifndef secFreeArray
#define secFreeArray(ptr, size)   \
  do {                            \
    _secFreeArray((ptr), (size)); \
    (ptr) = NULL;                 \
  } while (0)
#endif  // secFreeArray
#ifndef _secFreeMultiple
#define _secFreeMultiple(...)                                    \
  do {                                                           \
    _secFreeMultipleN(COUNT_VARARGS("dummy", ##__VA_ARGS__) - 1, \
                      ##__VA_ARGS__);                            \
  } while (0)
#endif  // _secFreeMultiple
#ifndef secFreeMultiple
#define secFreeMultiple(...)                        \
  do {                                              \
    _secFreeMultiple(NULL, ##__VA_ARGS__);          \
    CALL_MACRO_X_FOR_EACH(_NULL_IT, ##__VA_ARGS__); \
  } while (0)
#endif  // secFreeMultiple

#endif  // MEMORY_H
