#include "colors.h"

#include "memory.h"
#include "stringUtils.h"

#include <stdio.h>

/**
 * @brief prints a message in a specific color
 * @param out the FD where the message should be printed
 * @param colorCode the color code
 * @param fmt the format string of the message
 * @param args the arguments of the message
 * @return the return value of the @c vfprintf function
 */
int _vprintColored(FILE* out, char* colorCode, char* fmt, va_list args) {
  char* colored = oidc_sprintf("%s%s%s", colorCode, fmt, C_RESET);
  int   ret     = vfprintf(out, colored, args);
  secFree(colored);
  return ret;
}

/**
 * @brief prints an message colored in C_ERROR
 * @param fmt the format string of the message
 * @param args the arguments of the message
 * @return the return value of the @c _vprintColored function
 */
int printErrorColored(char* fmt, va_list args) {
  return _vprintColored(stderr, C_ERROR, fmt, args);
}

/**
 * @brief prints an message colored in C_PROMPT
 * @param fmt the format string of the message
 * @param args the arguments of the message
 * @return the return value of the @c _vprintColored function
 */
int printPromptColored(char* fmt, va_list args) {
  return _vprintColored(stderr, C_PROMPT, fmt, args);
}

/**
 * @brief prints an message colored in C_IMPORTANT
 * @param fmt the format string of the message
 * @param args the arguments of the message
 * @return the return value of the @c _vprintColored function
 */
int printImportantColored(char* fmt, va_list args) {
  return _vprintColored(stderr, C_IMPORTANT, fmt, args);
}
