#include "tc_strToULong.h"

#include "utils/stringUtils.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_strToULong() {
  TCase* tc = tcase_create("strToULong");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
