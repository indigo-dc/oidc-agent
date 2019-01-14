#include "tc_getDateString.h"

#include "utils/stringUtils.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_getDateString() {
  TCase* tc = tcase_create("getDateString");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
