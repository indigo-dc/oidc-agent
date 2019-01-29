#include "password_handler.h"

#include <stdlib.h>

char* getSavedPasswordFor(const char* shortname) { return NULL; }

char* getPasswordFromManagerFor(const char* shortname) { return NULL; }

char* getPasswordFromComandFor(const char* shortname) { return NULL; }

char* getPasswordFromPromptFor(const char* shortname) {
  // return promptForPassword(shortname, msg);
  return NULL;
}

char* getPasswordFor(const char* shortname) {
  char* pw = getSavedPasswordFor(shortname);
  if (pw) {
    return pw;
  }
  pw = getPasswordFromManagerFor(shortname);
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
  return NULL;
}
