#include "tc_strelim.h"

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

#include <string.h>

START_TEST(test_noFound) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str)];
  strcpy(s, str);
  ck_assert_msg(strcmp(strelim(s, 'x'), str) == 0,
                "did falsly eliminate characters");
}
END_TEST

START_TEST(test_elim) {
  const char* const str = "abcdeffedcbaabcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert(strcmp(strelim(s, 'b'), "acdeffedcaacdeffedca") == 0);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_msg(strelim(NULL, 'b') == NULL, "return value is not NULL");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

TCase* test_case_strelim() {
  TCase* tc = tcase_create("strelim");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_elim);
  tcase_add_test(tc, test_noFound);
  return tc;
}
