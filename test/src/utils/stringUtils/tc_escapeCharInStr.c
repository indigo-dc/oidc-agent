#include "tc_escapeCharInStr.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_NULL) {
  ck_assert_ptr_eq(escapeCharInStr(NULL, 'x'), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_notFound) {
  const char* const str = "abcdef";
  char*             s   = escapeCharInStr(str, 'x');
  ck_assert_ptr_ne(s, NULL);
  ck_assert_str_eq(s, str);
}
END_TEST

START_TEST(test_escape) {
  const char* const str = "abcdef";
  char*             s   = escapeCharInStr(str, 'c');
  ck_assert_ptr_ne(s, NULL);
  ck_assert_str_eq(s, "ab\\cdef");
}
END_TEST

TCase* test_case_escapeCharInStr() {
  TCase* tc = tcase_create("escapeCharInStr");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_notFound);
  tcase_add_test(tc, test_escape);
  return tc;
}
