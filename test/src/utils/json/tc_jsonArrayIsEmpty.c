#include "tc_jsonArrayIsEmpty.h"

#include "utils/json.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_jsonArrayIsEmpty() {
  TCase* tc = tcase_create("jsonArrayIsEmpty");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
