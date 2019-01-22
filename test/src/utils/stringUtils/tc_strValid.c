#include "tc_strValid.h"

#include "utils/stringUtils.h"

START_TEST(test_valid) { ck_assert(strValid("validString")); }
END_TEST

START_TEST(test_empty) { ck_assert(!strValid("")); }
END_TEST

START_TEST(test_NULL) { ck_assert(!strValid(NULL)); }
END_TEST

START_TEST(test_null) { ck_assert(!strValid("null")); }
END_TEST

START_TEST(test_null2) { ck_assert(!strValid("(null)")); }
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
