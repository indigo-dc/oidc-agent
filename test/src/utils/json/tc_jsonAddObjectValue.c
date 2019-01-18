#include "tc_jsonAddObjectValue.h"

#include "utils/json.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_jsonAddObjectValue() {
  TCase* tc = tcase_create("jsonAddObjectValue");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
