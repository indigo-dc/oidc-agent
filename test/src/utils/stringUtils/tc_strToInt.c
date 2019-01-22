#include "tc_strToInt.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_positive) { ck_assert_int_eq(strToInt("42"), 42); }
END_TEST

START_TEST(test_zero) { ck_assert_int_eq(strToInt("0"), 0); }
END_TEST

START_TEST(test_negative) { ck_assert_int_eq(strToInt("-42"), -42); }
END_TEST

START_TEST(test_NULL) {
  ck_assert_int_eq(strToInt(NULL), 0);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_strToInt() {
  TCase* tc = tcase_create("strToInt");
  tcase_add_test(tc, test_zero);
  tcase_add_test(tc, test_positive);
  tcase_add_test(tc, test_negative);
  tcase_add_test(tc, test_NULL);
  return tc;
}
