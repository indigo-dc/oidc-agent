#include "tempenv.h"

#include "defines/msys.h"
#include "defines/settings.h"

#ifndef ANY_MSYS
#include <stdlib.h>
#else
#include <ctype.h>
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
  char* p = getenv("TEMP");  // For some reason in cmd the userprofile
                             // part was cut in my tests, but apparently
                             // this is not always the case
  if (p == NULL) {
    return NULL;
  }
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