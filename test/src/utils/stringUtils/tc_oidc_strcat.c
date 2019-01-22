#include "tc_oidc_strcat.h"

#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_concat) {
  char* s = oidc_strcat("asdf", "1234");
  ck_assert_str_eq(s, "asdf1234");
  secFree(s);
}
END_TEST

START_TEST(test_NULL1) {
  char* s = oidc_strcat(NULL, "1234");
  ck_assert_ptr_eq(s, NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
  secFree(s);
}
END_TEST

START_TEST(test_NULL2) {
  char* s = oidc_strcat("asdf", NULL);
  ck_assert_ptr_eq(s, NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
  secFree(s);
}
END_TEST

TCase* test_case_oidc_strcat() {
  TCase* tc = tcase_create("oidc_strcat");
  tcase_add_test(tc, test_concat);
  tcase_add_test(tc, test_NULL1);
  tcase_add_test(tc, test_NULL2);
  return tc;
}
