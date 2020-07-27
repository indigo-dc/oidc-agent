#include "gen_consenter.h"

#include "utils/prompt.h"
#include "utils/prompt_mode.h"

int _gen_prompter(const char* prompt, const struct arguments* arguments,
                  int (*consentFnc)(const char*)) {
  if (arguments->confirm_yes) {
    return 1;
  }
  if (arguments->confirm_no) {
    return 0;
  }
  int oldMode = prompt_mode();
  if (!oldMode) {
    int newMode = pw_prompt_mode();  // If pw_prompt not set, it will return
                                     // PROMPT_MODE_CLI
    set_prompt_mode(newMode);
  }
  int ret = consentFnc(prompt);
  set_prompt_mode(oldMode);
  return ret;
}

int gen_promptConsentDefaultNo(const char*             text,
                               const struct arguments* arguments) {
  return _gen_prompter(text, arguments, promptConsentDefaultNo);
}

int gen_promptConsentDefaultYes(const char*             text,
                                const struct arguments* arguments) {
  return _gen_prompter(text, arguments, promptConsentDefaultYes);
}
