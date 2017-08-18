#include <string.h>
#include <stdlib.h>

#include "oidc_utilities.h"

/** @fn int isValid(const char* c)
 * @brief checks if a string contains a valid value, meaning it is not empty,
 * NULL or "(null)"
 * @param c the string to be checked
 * @return 1 if the string is valid; 0 if not
 */
int isValid(const char* c) {
  return c && strcmp("", c)!=0 && strcmp("(null)", c)!=0;
}

/** @fn strstarts(const char* str, const char* pre)
 * @brief checks if a string starts with a given string
 * @param str the string to be checked
 * @param pre the prefix \p str might start with
 * @return 1 if str starts with pre; 0 if not
 */
int strstarts(const char* str, const char* pre) {
  return strncmp(pre, str, strlen(pre)) == 0;
}


/** @fn void clearFree(void* p, size_t len)
 * @brief clears and frees allocated memory.
 * @param p a pointer to the memory to be freed
 * @param len the length of the allocated memory
 */
void clearFree(void* p, size_t len) {
  if(p==NULL) {
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
  if(s==NULL) {
    return;
  }
  clearFree(s, strlen(s));
}
