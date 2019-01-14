#include "tc_oidc_strncopy.h"

#include "utils/stringUtils.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_oidc_strncopy() {
  TCase* tc = tcase_create("oidc_strncopy");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
