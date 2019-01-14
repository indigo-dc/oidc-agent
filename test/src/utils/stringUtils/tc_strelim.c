#include "tc_strelim.h"

#include "utils/stringUtils.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_strelim() {
  TCase* tc = tcase_create("strelim");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
