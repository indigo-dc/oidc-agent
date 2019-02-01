#include "password_handler.h"
#include "internal_storage.h"
#include "keyring.h"
#include "utils/stringUtils.h"

#include <stdlib.h>

char* getPasswordFromComandFor(const char* shortname) { return NULL; }

char* getPasswordFromPromptFor(const char* shortname) {
  // return promptForPassword(shortname, msg);
  return NULL;
}

char* getPasswordFor(const char* shortname) {
  char* pw = internal_getPasswordFor(shortname);
  if (pw) {
    return pw;
  }
  pw = keyring_getPasswordFor(shortname);
  if (pw) {
    return pw;
  }
  pw = getPasswordFromComandFor(shortname);
  if (pw) {
    return pw;
  }
  pw = getPasswordFromPromptFor(shortname);
  if (pw) {
    return pw;
  }
  // return NULL;
  return oidc_strcopy("oidc");
}
