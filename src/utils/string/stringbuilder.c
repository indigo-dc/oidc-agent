#include "stringbuilder.h"

#include <string.h>

#include "utils/memory.h"
#include "utils/string/stringUtils.h"

struct str_builder {
  char*  str;
  size_t alloced;
  size_t len;
  size_t alloc_interval;
};

str_builder_t* str_builder_create(size_t alloc_interval) {
  str_builder_t* sb  = secAlloc(sizeof(str_builder_t));
  sb->alloc_interval = alloc_interval;
  sb->str            = secAlloc(sb->alloc_interval + 1);
  sb->alloced        = sb->alloc_interval;
  sb->len            = 0;
  return sb;
}

void secFree_str_builder(str_builder_t* sb) {
  if (sb == NULL) {
    return;
  }
  secFree(sb->str);
  secFree(sb);
}

/** @brief Ensure there is enough space for data being added plus a NULL
 * terminator.
 *
 * @param sb The str_builder
 * @param add_len The length that needs to be added *not* including a
 * NULL terminator.
 */
static void str_builder_ensure_space(str_builder_t* sb, size_t add_len) {
  if (sb == NULL || add_len == 0) {
    return;
  }

  if (sb->alloced >= sb->len + add_len) {
    return;
  }

  while (sb->alloced < sb->len + add_len) {
    /* Doubling growth strategy. */
    sb->alloced <<= 1;
    if (sb->alloced == 0) {
      /* Left shift of max bits will go to 0. An unsigned type set to
       * -1 will return the maximum possible size. However, we should
       *  have run out of memory well before we need to do this. Since
       *  this is the theoretical maximum total system memory we don't
       *  have a flag saying we can't grow any more because it should
       *  be impossible to get to this point. */
      sb->alloced--;
    }
  }
  sb->str = secRealloc(sb->str, sb->alloced + 1);
}

void str_builder_add_str(str_builder_t* sb, const char* str) {
  if (sb == NULL || str == NULL || *str == '\0') {
    return;
  }

  size_t len = strlen(str);

  str_builder_ensure_space(sb, len);
  memmove(sb->str + sb->len, str, len);
  sb->len += len;
}

void str_builder_add_char(str_builder_t* sb, char c) {
  if (sb == NULL) {
    return;
  }
  str_builder_ensure_space(sb, 1);
  sb->str[sb->len] = c;
  sb->len++;
}

void str_builder_add_int(str_builder_t* sb, int val) {
  if (sb == NULL) {
    return;
  }
  char* v = oidc_sprintf("%d", val);
  str_builder_add_str(sb, v);
  secFree(v);
}

size_t str_builder_len(const str_builder_t* sb) {
  if (sb == NULL) {
    return 0;
  }
  return sb->len;
}

char* str_builder_get_string(const str_builder_t* sb) {
  if (sb == NULL) {
    return NULL;
  }
  return oidc_strcopy(sb->str);
}
