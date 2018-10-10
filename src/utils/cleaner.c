#include "cleaner.h"
#include "../oidc_error.h"

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

void* secAlloc(size_t size) {
  if (size == 0) {
    return NULL;
  }
  size_t sizesize = sizeof(size);
  void*  p        = calloc(size + sizesize, 1);
  if (p == NULL) {
    oidc_errno = OIDC_EALLOC;
    syslog(LOG_AUTH | LOG_ALERT,
           "Memory alloc failed when trying to allocate %lu bytes", size);
    return NULL;
  }
  *(size_t*)p = size;
  return p + sizeof(size);
}

void* secRealloc(void* p, size_t size) {
  if (p == NULL) {
    return secAlloc(size);
  }
  if (size == 0) {
    secFree(p);
    return NULL;
  }
  size_t oldsize = *(size_t*)(p - sizeof(size_t));
  size_t movelen = oldsize < size ? oldsize : size;
  void*  newp    = secAlloc(size);
  if (newp == NULL) {
    return NULL;
  }
  memmove(newp, p, movelen);
  secFree(p);
  return newp;
}

// void clearFreeStringArray(char** arr, size_t size) {
//   size_t i;
//   for (i = 0; i < size; i++) { clearFreeString(arr[i]); }
//   clearFree(arr, size * sizeof(char*));
// }

void secFree(void* p) {
  void*  fp  = p - sizeof(size_t);
  size_t len = *(size_t*)fp;
  secFreeN(fp, len);
}
/** @fn void clearFree(void* p, size_t len)
 * @brief clears and frees allocated memory.
 * @param p a pointer to the memory to be freed
 * @param len the length of the allocated memory
 */
void secFreeN(void* p, size_t len) {
  if (p == NULL) {
    return;
  }
  memset(p, 0, len);
  free(p);
}

// /** @fn void clearFreeString(char* s)
//  * @brief clears and frees an allocated string.
//  * @param s a pointer to the string to be freed
//  */
// void clearFreeString(char* s) {
//   if (s == NULL) {
//     return;
//   }
//   clearFree(s, strlen(s));
// }
