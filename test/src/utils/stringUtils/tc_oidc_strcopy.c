#include "tc_oidc_strcopy.h"

#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_copy) {
  const char* const str = "someTestString";
  char*             s   = oidc_strcopy(str);
  ck_assert_str_eq(s, str);
  secFree(s);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_ptr_eq(oidc_strcopy(NULL), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_oidc_strcopy() {
  TCase* tc = tcase_create("oidc_strcopy");
  tcase_add_test(tc, test_copy);
  tcase_add_test(tc, test_NULL);
  return tc;
}
