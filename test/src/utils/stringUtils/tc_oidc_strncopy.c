#include "tc_oidc_strncopy.h"

#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <string.h>

START_TEST(test_copy) {
  const char* const str = "someTestString";
  char*             s   = oidc_strncopy(str, strlen(str));
  ck_assert(strcmp(str, s) == 0);
  secFree(s);
}
END_TEST

START_TEST(test_copyn) {
  char* s = oidc_strncopy("someTestString", 5);
  ck_assert_msg(strcmp("someT", s) == 0,
                "oidc_strncopy did not restrict string len");
  secFree(s);
}
END_TEST

START_TEST(test_copyBigLen) {
  const char* const str = "someTestString";
  char*             s   = oidc_strncopy(str, 9000);
  ck_assert(strcmp(str, s) == 0);
  secFree(s);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_msg(oidc_strncopy(NULL, 10) == NULL, "return value is not NULL");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_zeroLen) {
  ck_assert_msg(oidc_strncopy("anything", 0) == NULL,
                "return value is not NULL");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
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
