#include "tc_oidc_strcopy.h"

#include "utils/stringUtils.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_oidc_strcopy() {
  TCase* tc = tcase_create("oidc_strcopy");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
