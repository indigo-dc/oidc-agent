#include "tc_oidc_sprintf.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_sprintf) {
  const char* const fmt = "%s%d%lusomething%s";
  const char* const s1  = "string";
  const char* const s2  = "something";
  int               i   = 85192394;
  unsigned long     l   = -1;
  char*             os  = oidc_sprintf(fmt, s1, i, l, s2);
  char*             s   = calloc(sizeof(char), 1024);
  sprintf(s, fmt, s1, i, l, s2);
  ck_assert_str_eq(os, s);
  free(s);
  secFree(os);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_ptr_eq(oidc_sprintf(NULL), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_oidc_sprintf() {
  TCase* tc = tcase_create("oidc_sprintf");
  tcase_add_test(tc, test_sprintf);
  tcase_add_test(tc, test_NULL);
  return tc;
}
