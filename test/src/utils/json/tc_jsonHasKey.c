#include "tc_jsonHasKey.h"

#include "utils/json.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, "dummy"); }
END_TEST

TCase* test_case_jsonHasKey() {
  TCase* tc = tcase_create("jsonHasKey");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
