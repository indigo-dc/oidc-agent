#include "tc_strValid.h"

#include "utils/stringUtils.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_strValid() {
  TCase* tc = tcase_create("strValid");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
