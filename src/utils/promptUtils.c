#include "promptUtils.h"
#include "defines/settings.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/prompt.h"
#include "utils/stringUtils.h"

#include <stddef.h>

char* getEncryptionPassword(const char* forWhat, const char* suggestedPassword,
                            unsigned int max_pass_tries) {
  char*        encryptionPassword = NULL;
  unsigned int i;
  unsigned int max_tries =
      max_pass_tries == 0 ? MAX_PASS_TRIES : max_pass_tries;
  for (i = 0; i < max_tries; i++) {
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
