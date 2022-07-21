#include "guiChecker.h"

#include <stdlib.h>

#ifdef __APPLE__
#include "utils/memory.h"
#include "utils/string/stringUtils.h"
#include "utils/system_runner.h"
#endif

int GUIAvailable() {
#ifdef __MSYS__
  return 1;
#else
  int gui = NULL != getenv("DISPLAY");
#ifdef __APPLE__
  if (!gui) {
    char* mng = getOutputFromCommand("launchctl managername");
    gui       = strequal("Aqua", mng);
    secFree(mng);
  }
  return gui;
#else
  return gui;
#endif
#endif
}
