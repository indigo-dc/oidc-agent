#ifndef OIDC_PROMPTANDSET_HELPER_H
#define OIDC_PROMPTANDSET_HELPER_H

#include "utils/printer.h"
#include "utils/prompt_mode.h"

#include <stdlib.h>

#ifndef ERROR_MESSAGE
#define ERROR_MESSAGE(_name, _option)                      \
  "Required argument " _name                               \
  " not given. Please enable a prompt mode or pass " _name \
  " to the '--" _option "' option."
#endif

#ifndef ERROR_IF_NO_PROMPT
#define ERROR_IF_NO_PROMPT(_optional, _error_message) \
  do {                                                \
    if (prompt_mode() == 0) {                         \
      if ((_optional)) {                              \
        return;                                       \
      }                                               \
      printError("%s\n", (_error_message));           \
      exit(EXIT_FAILURE);                             \
    }                                                 \
  } while (0)
#endif

char* _gen_prompt(char* label, const char* init, int passPrompt, int optional);
char* _gen_promptMultipleSpaceSeparated(char* label, const char* init_str,
                                        int optional);

#endif  // OIDC_PROMPTANDSET_HELPER_H
