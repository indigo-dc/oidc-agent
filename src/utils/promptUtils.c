#include "promptUtils.h"
#include "defines/settings.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/prompt.h"
#include "utils/stringUtils.h"
#include "utils/system_runner.h"

#include <stddef.h>

char* getEncryptionPasswordForAccountConfig(const char*   shortname,
                                            const char*   suggestedPassword,
                                            const char*   pw_cmd,
                                            unsigned int  max_pass_tries,
                                            unsigned int* number_try) {
  char* forWhat = oidc_sprintf("account config '%s'", shortname);
  char* ret     = getEncryptionPasswordFor(forWhat, suggestedPassword, pw_cmd,
                                       max_pass_tries, number_try);
  secFree(forWhat);
  return ret;
}

char* getEncryptionPasswordFor(const char* forWhat,
                               const char* suggestedPassword,
                               const char* pw_cmd, unsigned int max_pass_tries,
                               unsigned int* number_try) {
  unsigned int max_tries =
      max_pass_tries == 0 ? MAX_PASS_TRIES : max_pass_tries;
  if (pw_cmd && (number_try == NULL || *number_try == 0)) {
    if (number_try) {
      (*number_try)++;
    }
    char* pass = getOutputFromCommand(pw_cmd);
    if (pass) {
      return pass;
    }
  }
  char* encryptionPassword = NULL;
  for (; number_try == NULL || *number_try < max_tries; (*number_try)++) {
    char* input =
        promptPassword("Enter encryption password for %s%s: ", forWhat,
                       strValid(suggestedPassword) ? " [***]" : "");
    if (strValid(suggestedPassword) &&
        !strValid(input)) {  // use same encryption password
      secFree(input);
      encryptionPassword = oidc_strcopy(suggestedPassword);
      return encryptionPassword;
    } else {
      encryptionPassword = input;
      char* confirm      = promptPassword("Confirm encryption Password: ");
      if (!strequal(encryptionPassword, confirm)) {
        printError("Encryption passwords did not match.\n");
        secFree(confirm);
        secFree(encryptionPassword);
      } else {
        secFree(confirm);
        return encryptionPassword;
      }
    }
  }
  if (encryptionPassword) {
    secFree(encryptionPassword);
  }

  oidc_errno = OIDC_EMAXTRIES;
  return NULL;
}
