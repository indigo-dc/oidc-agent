#include "tc_mergeJSONObjectStrings.h"

#include "utils/json.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_mergeJSONObjectStrings() {
  TCase* tc = tcase_create("mergeJSONObjectStrings");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
