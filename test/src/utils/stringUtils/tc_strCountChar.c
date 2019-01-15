#include "tc_strCountChar.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_NULL) {
  ck_assert_msg(strCountChar(NULL, 'b') == 0, "return value is not 0");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_notFound) { ck_assert(strCountChar("test", 'b') == 0); }
END_TEST

START_TEST(test_found) { ck_assert(strCountChar("test", 't') == 2); }
END_TEST

TCase* test_case_strCountChar() {
  TCase* tc = tcase_create("strCountChar");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_found);
  tcase_add_test(tc, test_notFound);
  return tc;
}
