#include "_helper.h"

#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/prompt.h"
#include "utils/string/stringUtils.h"

char* _gen_prompt(char* label, const char* init, int passPrompt, int optional) {
  char*     input    = NULL;
  promptFnc prompter = prompt;
  if (passPrompt) {
    prompter = promptPassword;
  }
  do {
    char* text = oidc_sprintf("Please enter %s:", label);
    input      = prompter(text, label, init, CLI_PROMPT_NOT_VERBOSE);
    secFree(text);
    if (strValid(input)) {
      return input;
    }
    secFree(input);
    if (optional) {
      return NULL;
    }
  } while (1);
}

char* _gen_promptMultipleSpaceSeparated(char* label, const char* init_str,
                                        int optional) {
  list_t* init  = delimitedStringToList(init_str, ' ');
  list_t* input = NULL;
  do {
    char* text = oidc_sprintf("Please enter %s:", label);
    input      = promptMultiple(text, label, init, CLI_PROMPT_NOT_VERBOSE);
    secFree(text);
    if (listValid(input)) {
      char* output = listToDelimitedString(input, " ");
      logger(DEBUG,
             "In %s produced output '%s' for label '%s' with %lu elements",
             __func__, output, label, input->len);
      secFreeList(input);
      secFreeList(init);
      return output;
    }
    secFreeList(input);
    if (optional) {
      secFreeList(init);
      return NULL;
    }
  } while (1);
}
