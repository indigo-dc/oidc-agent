#include "tc__secFreeJson.h"

#include "utils/json.h"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, "dummy"); }
END_TEST

TCase* test_case__secFreeJson() {
  TCase* tc = tcase_create("_secFreeJson");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
