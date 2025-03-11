#include "oidcUtils.h"

#include <string.h>

#include "utils/string/stringUtils.h"

char* removeScope(char* scopes, const char* rem) {
  scopes = strremove(scopes, rem);
  scopes = strelimIfAfter(
      scopes, ' ',
      ' ');  // strremove leaves a doubled space; this call removes one of it.
  return scopes;
}
