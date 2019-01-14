#include "tc_strValid.h"

#include "utils/stringUtils.h"

START_TEST(test_valid) {
  ck_assert_msg(strValid("validString"), "A valid string was not valid");
}
END_TEST

START_TEST(test_empty) {
  ck_assert_msg(!strValid(""), "Empty string falsly valid");
}
END_TEST

START_TEST(test_NULL) { ck_assert_msg(!strValid(NULL), "NULL falsly valid"); }
END_TEST

START_TEST(test_null) {
  ck_assert_msg(!strValid("null"), "'null' falsly valid");
}
END_TEST

START_TEST(test_null2) {
  ck_assert_msg(!strValid("(null)"), "'(null)' falsly valid");
}
END_TEST

TCase* test_case_strValid() {
  TCase* tc = tcase_create("strValid");
  tcase_add_test(tc, test_valid);
  tcase_add_test(tc, test_empty);
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_null);
  tcase_add_test(tc, test_null2);
  return tc;
}
