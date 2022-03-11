#define _XOPEN_SOURCE
#include "printer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/colors.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

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

int isTTY(FILE* out) { return isatty(fileno(out)); }

int getColorSupportStderr() { return getColorSupportFor(stderr); }

int getColorSupportStdout() { return getColorSupportFor(stdout); }

int printNormal(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  int   ret = vfprintf(out, fmt, args);
  va_end(args);
  return ret;
}

int fprintNormal(FILE* out, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vfprintf(out, fmt, args);
  va_end(args);
  return ret;
}

int printStdout(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vprintf(fmt, args);
  va_end(args);
  return ret;
}

int printError(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  int   ret;
  if (getColorSupportFor(out)) {
    ret = printErrorColored(fmt, args);
  } else {
    ret = vfprintf(out, fmt, args);
  }
  va_end(args);
  return ret;
}

int printPrompt(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  int   ret;
  if (getColorSupportFor(out)) {
    ret = printPromptColored(fmt, args);
  } else {
    ret = vfprintf(out, fmt, args);
  }
  va_end(args);
  return ret;
}

int printImportant(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  FILE* out = stderr;
  int   ret;
  if (getColorSupportFor(out)) {
    ret = printImportantColored(fmt, args);
  } else {
    ret = vfprintf(out, fmt, args);
  }
  va_end(args);
  return ret;
}

int printNormalIfTTY(char* fmt, ...) {
  FILE* out = stderr;
  if (!isTTY(out)) {
    return 0;
  }
  va_list args;
  va_start(args, fmt);
  int ret = vfprintf(out, fmt, args);
  va_end(args);
  return ret;
}

int fprintNormalIfTTY(FILE* out, char* fmt, ...) {
  if (!isTTY(out)) {
    return 0;
  }
  va_list args;
  va_start(args, fmt);
  int ret = vfprintf(out, fmt, args);
  va_end(args);
  return ret;
}

int printStdoutIfTTY(char* fmt, ...) {
  if (!isTTY(stdout)) {
    return 0;
  }
  va_list args;
  va_start(args, fmt);
  int ret = vprintf(fmt, args);
  va_end(args);
  return ret;
}
