#include "stringUtils.h"

#include "utils/memory.h"

char* repeatChar(char c, size_t n) {
  char* str = secAlloc(n + 1);
  memset(str, c, n);
  return str;
}