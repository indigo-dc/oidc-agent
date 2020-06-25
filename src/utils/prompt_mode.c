#include "prompt_mode.h"

#include "utils/oidc_error.h"

#include <stdlib.h>

unsigned char _prompt_mode;
unsigned char _pw_prompt_mode;

void set_prompt_mode(unsigned char mode) {
  _prompt_mode = mode;
  set_pw_prompt_mode(mode);
}
void set_pw_prompt_mode(unsigned char mode) { _pw_prompt_mode = mode; }

unsigned char prompt_mode() { return _prompt_mode; }
unsigned char pw_prompt_mode() { return _pw_prompt_mode; }
