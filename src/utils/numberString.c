#include "utils/numberString.h"

#include <string.h>

#include "utils/memzero.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

static char table[] = " !\"#$%&'()*+,-./"
                      "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                      "abcdefghijklmnopqrstuvwxyz{|}~";

unsigned short charToNumber(char c) {
  for (size_t i = 0; i < strlen(table); i++) {
    if (table[i] == c) {
      return (unsigned short)(i + 1);
    }
  }
  return -1;
}

char numberToChar(unsigned short s) { return table[s - 1]; }

unsigned long long lpow(unsigned long long base, unsigned long long exp) {
  unsigned long long result = 1ULL;
  while (exp) {
    if (exp & 1) {
      result *= base;
    }
    exp >>= 1;
    base *= base;
  }
  return result;
}

// Do not use more than 9 characters
unsigned long long stringToNumber(char* str) {
  if (str == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  unsigned long long i = 0;
  while (*str != '\0') {
    i += lpow(strlen(table) + 1, (strlen(str) - 1)) * charToNumber(*str);
    str++;
  }
  return i;
}

char* numberToString(unsigned long long l) {
  char  str[10];
  short i = sizeof(str) - 1;
  str[i]  = '\0';
  while (l && i >= 0) {
    i--;
    str[i] = numberToChar(l % (strlen(table) + 1));
    l /= strlen(table) + 1;
  }
  char* ret = oidc_strcopy(&str[i]);
  moresecure_memzero(str, 10);
  return ret;
}
