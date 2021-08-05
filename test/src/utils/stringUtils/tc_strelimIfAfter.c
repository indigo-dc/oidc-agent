#include "tc_strelimIfAfter.h"

#include <string.h>

#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

START_TEST(test_noElim) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelimIfAfter(s, 'b', 'x'), str);
}
END_TEST

START_TEST(test_noFound) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelimIfAfter(s, 'x', 'a'), str);
}
END_TEST

START_TEST(test_elim) {
  const char* const str = "abcdeffedcbaabcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelimIfAfter(s, 'b', 'c'), "abcdeffedcaabcdeffedca");
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_ptr_eq(strelimIfAfter(NULL, 'b', 'c'), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_strelimIfAfter() {
  TCase* tc = tcase_create("strelimIfAfter");
  tcase_add_test(tc, test_noElim);
  tcase_add_test(tc, test_noFound);
  tcase_add_test(tc, test_elim);
  tcase_add_test(tc, test_NULL);
  return tc;
}
