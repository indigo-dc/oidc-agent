#include "tc_strToULong.h"

#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

START_TEST(test_positive) { ck_assert_uint_eq(strToULong("42"), 42); }
END_TEST

START_TEST(test_zero) { ck_assert_uint_eq(strToULong("0"), 0); }
END_TEST

START_TEST(test_negative) {
  ck_assert_uint_eq(strToULong("-42"), (unsigned long)-42);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_uint_eq(strToULong(NULL), 0);
  ck_assert_uint_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_strToULong() {
  TCase* tc = tcase_create("strToULong");
  tcase_add_test(tc, test_zero);
  tcase_add_test(tc, test_positive);
  tcase_add_test(tc, test_negative);
  tcase_add_test(tc, test_NULL);
  return tc;
}
