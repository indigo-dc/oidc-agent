#define _XOPEN_SOURCE
#include "printer.h"

#include "colors.h"
#include "memory.h"
#include "stringUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int getColorSupport() {
  char* agent_color = NULL;
  if ((agent_color = getenv("OIDC_AGENT_NOCOLOR")) != NULL) {
    if (*agent_color ==
        '0') {  // If NOCOLOR is false for OIDC-AGENT, color is desired
      return 1;
    } else {  // if set to any other value, don't use color
      return 0;
    }
  }
  if (getenv("NO_COLOR")) {  // If $NO_COLOR is set, don't use color
    return 0;
  }
  if (strequal(getenv("TERM"),
               "dumb")) {  // If $TERM is 'dumb', don't use color
    return 0;
  }
  return 1;
}

int getColorSupportFor(FILE* out) {
  return getColorSupport() && isatty(fileno(out));
}

int getColorSupportStderr() { return getColorSupportFor(stderr); }

int getColorSupportStdout() { return getColorSupportFor(stdout); }

int printNormal(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  return vfprintf(out, fmt, args);
}

int fprintNormal(FILE* out, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return vfprintf(out, fmt, args);
}

int printError(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  if (getColorSupportFor(out)) {
    return printErrorColored(fmt, args);
  }
  return vfprintf(out, fmt, args);
}

int printPrompt(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  if (getColorSupportFor(out)) {
    return printPromptColored(fmt, args);
  }
  return vfprintf(out, fmt, args);
}

int printImportant(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  if (getColorSupportFor(out)) {
    return printImportantColored(fmt, args);
  }
  return vfprintf(out, fmt, args);
}
