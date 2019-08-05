#include "tc_cjose_getSignHeader.h"

#include "oidc-agent/oidcd/jose/joseUtils.h"

START_TEST(test_null) { ck_assert_ptr_eq(cjose_getSignHeader(NULL), NULL); }
END_TEST

START_TEST(test_ok) { ck_assert_ptr_ne(cjose_getSignHeader("RS256"), NULL); }
END_TEST

TCase* test_case_cjose_getSignHeader() {
  TCase* tc = tcase_create("cjose_getSignHeader");
  tcase_add_test(tc, test_null);
  tcase_add_test(tc, test_ok);
  return tc;
}
