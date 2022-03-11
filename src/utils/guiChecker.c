#include "guiChecker.h"

#include <stdlib.h>

int GUIAvailable() {
#ifdef __MSYS__
  return 1;
#else
  return NULL != getenv("DISPLAY");
#endif
}
