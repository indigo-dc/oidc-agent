#include "tc_strelim.h"

#include <string.h>

#include "utils/oidc_error.h"
#include "utils/stringUtils.h"

START_TEST(test_noFound) {
  const char* const str = "abcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelim(s, 'x'), str);
}
END_TEST

START_TEST(test_elim) {
  const char* const str = "abcdeffedcbaabcdeffedcba";
  char              s[strlen(str) + 1];
  strcpy(s, str);
  ck_assert_str_eq(strelim(s, 'b'), "acdeffedcaacdeffedca");
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_ptr_eq(strelim(NULL, 'b'), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

TCase* test_case_strelim() {
  TCase* tc = tcase_create("strelim");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_elim);
  tcase_add_test(tc, test_noFound);
  return tc;
}
