#include "tc_oidc_strcat.h"

#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_concat) {
  char* s = oidc_strcat("asdf", "1234");
  ck_assert(strcmp(s, "asdf1234") == 0);
  secFree(s);
}
END_TEST

START_TEST(test_NULL1) {
  char* s = oidc_strcat(NULL, "1234");
  ck_assert_msg(s == NULL, "return value not NULL");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
  secFree(s);
}
END_TEST

START_TEST(test_NULL2) {
  char* s = oidc_strcat("asdf", NULL);
  ck_assert_msg(s == NULL, "return value not NULL");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
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
