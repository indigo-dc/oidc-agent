#include "tempenv.h"

#include "defines/msys.h"
#include "defines/settings.h"

#ifndef ANY_MSYS
#include <stdlib.h>
#else
#include <ctype.h>
#include <string.h>
#include <windows.h>

#include "utils/logger.h"
#include "utils/string/stringUtils.h"

#define B_SIZE 2048
char full_win_path[B_SIZE] = {0};

// win_path_equal compares two path(components) up to n bytes; '/' and '\' are
// considered equal
static unsigned char win_path_n_equal(const char* a, const char* b, size_t n) {
  if (strlen(a) < n) {
    n = strlen(a);
  }
  if (strlen(b) < n) {
    n = strlen(b);
  }
  for (size_t i = 0; i < n; i++) {
    if (a[i] == b[i]) {
      continue;
    }
    if ((a[i] == '/' || a[i] == '\\') && (b[i] == '/' || b[i] == '\\')) {
      continue;
    }
    return 0;
  }
  return 1;
}
static const char* melt_paths_that_might_overlap(const char* a, const char* b) {
  // we melt the paths a and b together, but if a ends with path components that
  // b starts with they are not duplicated; we do this because of weird windows
  // msys stuff; there certainly can be situations where this is not desired
  // Examples:

  // C:\Users\janedoe + /AppData/Local/Temp ->
  // C:\Users\janedoe/AppData/Local/Temp

  // C:\Users\janedoe + Users/janedoe/AppData/Local/Temp ->
  // C:\Users\janedoe/AppData/Local/Temp C:/Users/janedoe +

  size_t overlap_len = strlen(a);
  if (strlen(b) < overlap_len) {
    overlap_len = strlen(b);
  }
  for (size_t i = overlap_len - strlen(a); overlap_len > 0;
       i++, overlap_len--) {
    if (win_path_n_equal(a + i, b, overlap_len)) {
      strncpy(full_win_path, a, B_SIZE);
      strncpy(full_win_path + strlen(a), b + overlap_len, B_SIZE - strlen(a));
      full_win_path[B_SIZE - 1] = '\0';
      return full_win_path;
    }
  }
  strncpy(full_win_path, a, B_SIZE);
  strncpy(full_win_path + strlen(a), b, B_SIZE - strlen(a));
  full_win_path[B_SIZE - 1] = '\0';
  return full_win_path;
}

#endif

const char* get_tmp_env() {
#ifndef ANY_MSYS
  return getenv(TMPDIR_ENVVAR);
#else
  // In the msys terminals $TEMP is '/tmp' and only $temp is the correct windows
  // tmp dir location. But in windows cmd/ps getenv("temp") does not give us
  // anything, only getenv("TEMP")
  if (getenv("MSYSTEM") != NULL) {  // MSYS
    return getenv("temp");
  }
  // windows
  logger(DEBUG,
         "We are in a windows non-msys terminal and determine the tmp dir");
  char* p = getenv("TEMP");  // For some reason in cmd the userprofile
                             // part was cut in my tests, but apparently
                             // this is not always the case
  if (p == NULL) {
    return NULL;
  }
  logger(DEBUG, "$TEMP is '%s'", p);
  if (p[0] != '/') {
    return p;
  }
  if (strstarts(p, "/cygdrive") && strlen(p) > 10) {
    // expects that path has the form /cygdrive/c/Users/<user>/...
    // We want to transform it into C:/Users/<user>/...
    if (p[9] == '/') {  // The first time we call this the string has the form
                        // /cygdrive/c/... but then we edit the string in the
                        // env (of this programm) so on future call it is
                        // already /cygdriveC:/... so don't do any
                        // transformation, just return the relevant part
      p[9] = toupper(p[10]);  // capitalize drive letter and move it one
                              // position to the front
      p[10] = ':';
    }
    return p + 9;
  }
  const char* up = getenv("USERPROFILE");
  logger(DEBUG, "$USERPROFILE is '%s'", up);
  size_t up_len = strlen(up);
  if (up_len + strlen(p) >= B_SIZE) {
    return NULL;
  }
  return melt_paths_that_might_overlap(up, p);
#endif
}