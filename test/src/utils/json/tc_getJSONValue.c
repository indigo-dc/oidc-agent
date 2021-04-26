#include "tc_getJSONValue.h"

#include "utils/json.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, "dummy"); }
END_TEST

TCase* test_case_getJSONValue() {
  TCase* tc = tcase_create("getJSONValue");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
