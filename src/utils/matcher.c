#include "matcher.h"
#include "account/issuer_helper.h"
#include "utils/stringUtils.h"

#include <stddef.h>

int matchStrings(const char* a, const char* b) {
  if (a == NULL && b == NULL) {
    return 1;
  }
  if (a == NULL) {
    return 0;
  }
  if (b == NULL) {
    return 0;
  }
  return strequal(a, b);
}

int matchUrls(const char* a, const char* b) {
  if (a == NULL && b == NULL) {
    return 1;
  }
  if (a == NULL) {
    return 0;
  }
  if (b == NULL) {
    return 0;
  }
  return compIssuerUrls(a, b);
}
