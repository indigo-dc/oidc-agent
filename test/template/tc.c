#include "tc_$NORMAL$.h"

#include "$SRCHEADER$"

// TODO
START_TEST(test_) { ck_assert_msg(1 == 1, ""); }
END_TEST

TCase* test_case_$NORMAL$() {
  TCase* tc = tcase_create("$NORMAL$");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
