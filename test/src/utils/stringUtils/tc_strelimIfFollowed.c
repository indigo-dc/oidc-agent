#include "tc_strelimIfFollowed.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <string.h>

START_TEST(test_noElim) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelimIfFollowed(s, 'b', 'x'), str);
}
END_TEST

START_TEST(test_noFound) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelimIfFollowed(s, 'x', 'a'), str);
}
END_TEST

START_TEST(test_elim) {
  const char* const str = "abcdeffedcbaabcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelimIfFollowed(s, 'b', 'c'), "acdeffedcbaacdeffedcba");
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_ptr_eq(strelimIfFollowed(NULL, 'b', 'c'), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_strelimIfFollowed() {
  TCase* tc = tcase_create("strelimIfFollowed");
  tcase_add_test(tc, test_noElim);
  tcase_add_test(tc, test_noFound);
  tcase_add_test(tc, test_elim);
  tcase_add_test(tc, test_NULL);
  return tc;
}
