#include "tc_strelimIfAfter.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <string.h>

START_TEST(test_noElim) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str)];
  strcpy(s, str);
  ck_assert_msg(strcmp(strelimIfAfter(s, 'b', 'x'), str) == 0,
                "did falsly eliminate characters");
}
END_TEST

START_TEST(test_noFound) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str)];
  strcpy(s, str);
  ck_assert_msg(strcmp(strelimIfAfter(s, 'x', 'a'), str) == 0,
                "did falsly elimanate characters");
}
END_TEST

START_TEST(test_elim) {
  const char* const str = "abcdeffedcbaabcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert(strcmp(strelimIfAfter(s, 'b', 'c'), "abcdeffedcaabcdeffedca") == 0);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_msg(strelimIfAfter(NULL, 'b', 'c') == NULL,
                "return value is not NULL");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

TCase* test_case_strelimIfAfter() {
  TCase* tc = tcase_create("strelimIfAfter");
  tcase_add_test(tc, test_noElim);
  tcase_add_test(tc, test_noFound);
  tcase_add_test(tc, test_elim);
  tcase_add_test(tc, test_NULL);
  return tc;
}
