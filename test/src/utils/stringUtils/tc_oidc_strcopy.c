#include "tc_oidc_strcopy.h"

#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_copy) {
  const char* const str = "someTestString";
  char*             s   = oidc_strcopy(str);
  ck_assert(strcmp(str, s) == 0);
  secFree(s);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_msg(oidc_strcopy(NULL) == NULL, "return value is not NULL");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

TCase* test_case_oidc_strcopy() {
  TCase* tc = tcase_create("oidc_strcopy");
  tcase_add_test(tc, test_copy);
  tcase_add_test(tc, test_NULL);
  return tc;
}
