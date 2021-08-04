#include "guiChecker.h"

#include <stdlib.h>

int GUIAvailable() { return NULL != getenv("DISPLAY"); }