#include "tc_strEnds.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_ends) { ck_assert(strEnds("testSuffix", "Suffix")); }
END_TEST

START_TEST(test_notend) { ck_assert(!strEnds("testSufix", "Suffix")); }
END_TEST

START_TEST(test_emptySuffix) { ck_assert(strEnds("testString", "")); }
END_TEST

START_TEST(test_emptyString) { ck_assert(!strEnds("", "suffix")); }
END_TEST

START_TEST(test_bothEmpty) { ck_assert(strEnds("", "")); }
END_TEST

START_TEST(test_suffixNULL) {
  ck_assert_msg(!strEnds("anything", NULL), "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_stringNULL) {
  ck_assert_msg(!strEnds(NULL, "anything"), "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

TCase* test_case_strEnds() {
  TCase* tc = tcase_create("strEnds");
  tcase_add_test(tc, test_ends);
  tcase_add_test(tc, test_notend);
  tcase_add_test(tc, test_emptySuffix);
  tcase_add_test(tc, test_emptyString);
  tcase_add_test(tc, test_bothEmpty);
  tcase_add_test(tc, test_suffixNULL);
  tcase_add_test(tc, test_stringNULL);
  return tc;
}
