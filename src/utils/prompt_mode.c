#include "prompt_mode.h"

#include "utils/oidc_error.h"

#include <stdlib.h>

unsigned char _prompt_mode;

void set_prompt_mode(unsigned char mode) { _prompt_mode = mode; }

unsigned char prompt_mode() { return _prompt_mode; }
