#include "tc_strstarts.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_starts) { ck_assert(strstarts("prefixxxTest", "prefix")); }
END_TEST

START_TEST(test_notstart) { ck_assert(!strstarts("pefixxxTest", "prefix")); }
END_TEST

START_TEST(test_emptyPrefix) { ck_assert(strstarts("testString", "")); }
END_TEST

START_TEST(test_emptyString) { ck_assert(!strstarts("", "prefix")); }
END_TEST

START_TEST(test_bothEmpty) { ck_assert(strstarts("", "")); }
END_TEST

START_TEST(test_prefixNULL) {
  ck_assert(!strstarts("anything", NULL));
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_stringNULL) {
  ck_assert(!strstarts(NULL, "anything"));
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_strstarts() {
  TCase* tc = tcase_create("strstarts");
  tcase_add_test(tc, test_starts);
  tcase_add_test(tc, test_notstart);
  tcase_add_test(tc, test_emptyPrefix);
  tcase_add_test(tc, test_emptyString);
  tcase_add_test(tc, test_bothEmpty);
  tcase_add_test(tc, test_prefixNULL);
  tcase_add_test(tc, test_stringNULL);
  return tc;
}
