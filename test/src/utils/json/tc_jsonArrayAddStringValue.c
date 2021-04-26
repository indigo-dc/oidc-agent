#include "tc_jsonArrayAddStringValue.h"

#include "utils/json.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, "dummy"); }
END_TEST

TCase* test_case_jsonArrayAddStringValue() {
  TCase* tc = tcase_create("jsonArrayAddStringValue");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
