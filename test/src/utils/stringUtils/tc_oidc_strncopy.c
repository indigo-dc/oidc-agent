#include "tc_oidc_strncopy.h"

#include <string.h>

#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_copy) {
  const char* const str = "someTestString";
  char*             s   = oidc_strncopy(str, strlen(str));
  ck_assert_str_eq(s, str);
  secFree(s);
}
END_TEST

START_TEST(test_copyn) {
  char* s = oidc_strncopy("someTestString", 5);
  ck_assert_str_eq("someT", s);
  secFree(s);
}
END_TEST

START_TEST(test_copyBigLen) {
  const char* const str = "someTestString";
  char*             s   = oidc_strncopy(str, 9000);
  ck_assert_str_eq(s, str);
  secFree(s);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_ptr_eq(oidc_strncopy(NULL, 10), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_zeroLen) {
  ck_assert_ptr_eq(oidc_strncopy("anything", 0), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_oidc_strncopy() {
  TCase* tc = tcase_create("oidc_strncopy");
  tcase_add_test(tc, test_copy);
  tcase_add_test(tc, test_copyn);
  tcase_add_test(tc, test_copyBigLen);
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_zeroLen);
  return tc;
}
