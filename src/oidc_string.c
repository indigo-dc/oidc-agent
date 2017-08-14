#include <string.h>

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


