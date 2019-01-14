#include "tc_oidc_strcat.h"

#include "utils/stringUtils.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_oidc_strcat() {
  TCase* tc = tcase_create("oidc_strcat");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
