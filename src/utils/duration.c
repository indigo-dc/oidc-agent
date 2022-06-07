#include "duration.h"

#include <ctype.h>
#include <string.h>
#include <time.h>

#include "utils/string/stringUtils.h"

unsigned int _unitMap(const char u) {
  switch (u) {
    case 's': return 1;
    case 'm': return 60;
    case 'h': return 60 * 60;
    case 'd': return 60 * 60 * 24;
    case 'w': return 60 * 60 * 24 * 7;
    case 'y': return 60 * 60 * 24 * 365;
    default: return 0;
  }
}

#define DSTATE_PRE 0
#define DSTATE_POST 1

time_t parseDuration(const char* str) {
  if (str == NULL) {
    return 0;
  }
  const char*   p = str;
  time_t        t = 0;
  unsigned long v = 0, n = 0;  // v integers before, n after decimal point
  unsigned int  scale = 1;
  unsigned char state = DSTATE_PRE;
  while (*p) {
    if (isdigit(*p)) {
      if (DSTATE_PRE == state) {
        v *= 10;
        v += *p - '0';
      } else if (DSTATE_POST == state) {
        n *= 10;
        n += *p - '0';
        scale *= 10;
      }
    } else if (*p == '.') {
      if (DSTATE_PRE != state) {
        return 0;
      }
      state = DSTATE_POST;
    } else {  // unit
      unsigned int f = _unitMap(*p);
      t += (time_t)(((double)v + (double)n / scale) * f);
      v = 0, n = 0, scale = 1;
      state = DSTATE_PRE;
    }
    p++;
  }
  return t;
}

time_t parseTime(const char* str) {
  if (str == NULL || strlen(str) == 0) {
    return 0;
  }
  time_t t = strToULong(str);
  if (t) {
    if (str[0] == '+') {
      return time(NULL) + t;
    }
    return t;
  }
  if (str[0] == '+') {
    time_t d = parseDuration(str + 1);
    return time(NULL) + d;
  }
  return parseDateStr(str);
}