#include "colors.h"

#include "memory.h"
#include "stringUtils.h"

#include <stdio.h>

int _vprintColored(FILE* out, char* colorCode, char* fmt, va_list args) {
  char* colored = oidc_sprintf("%s%s%s", colorCode, fmt, C_RESET);
  int   ret     = vfprintf(out, colored, args);
  secFree(colored);
  return ret;
}

int printErrorColored(char* fmt, va_list args) {
  return _vprintColored(stderr, C_ERROR, fmt, args);
}

int printPromptColored(char* fmt, va_list args) {
  return _vprintColored(stderr, C_PROMPT, fmt, args);
}

int printImportantColored(char* fmt, va_list args) {
  return _vprintColored(stderr, C_IMPORTANT, fmt, args);
}
