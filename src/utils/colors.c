#include "colors.h"

#include "../settings.h"
#include "cleaner.h"
#include "stringUtils.h"

#include <stdarg.h>
#include <stdio.h>

int printError(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* colored = oidc_sprintf("%s%s%s", C_ERROR, fmt, C_RESET);
  int ret = vfprintf(stderr, colored, args);
  clearFreeString(colored);
  return ret;
}

