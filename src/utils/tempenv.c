#include "tempenv.h"

#include "defines/msys.h"
#include "defines/settings.h"

#ifndef ANY_MSYS
#include <stdlib.h>
#else
#include <string.h>
#include <windows.h>

#include "utils/string/stringUtils.h"

#define B_SIZE 2048
char full_win_path[B_SIZE] = {0};
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
  const char* p = getenv("TEMP");  // For some reason in cmd the userprofile
                                   // part was cut in my tests, but apparently
                                   // this is not always the case
  if (p == NULL) {
    return NULL;
  }
  if (p[0] != '/') {
    return p;
  }
  const char* up     = getenv("USERPROFILE");
  size_t      up_len = strlen(up);
  if (up_len + strlen(p) >= B_SIZE) {
    return NULL;
  }
  strncpy(full_win_path, up, B_SIZE);
  strncpy(full_win_path + up_len, p, B_SIZE - up_len);
  full_win_path[B_SIZE - 1] = '\0';
  return full_win_path;
#endif
}