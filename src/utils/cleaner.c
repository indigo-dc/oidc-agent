#include "cleaner.h"

#include <stdlib.h>
#include <string.h>

void clearFreeStringArray(char** arr, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) { clearFreeString(arr[i]); }
  clearFree(arr, size * sizeof(char*));
}

/** @fn void clearFree(void* p, size_t len)
 * @brief clears and frees allocated memory.
 * @param p a pointer to the memory to be freed
 * @param len the length of the allocated memory
 */
void clearFree(void* p, size_t len) {
  if (p == NULL) {
    return;
  }
  memset(p, 0, len);
  free(p);
}

/** @fn void clearFreeString(char* s)
 * @brief clears and frees an allocated string.
 * @param s a pointer to the string to be freed
 */
void clearFreeString(char* s) {
  if (s == NULL) {
    return;
  }
  clearFree(s, strlen(s));
}
