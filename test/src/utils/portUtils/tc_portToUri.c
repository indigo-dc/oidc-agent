#include "tc_portToUri.h"

#include "utils/portUtils.h"

START_TEST(test_4242) {
  ck_assert_str_eq(portToUri(4242), "http://localhost:4242");
}
END_TEST

START_TEST(test_0) {
  ck_assert_ptr_eq(portToUri(0), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EPORTRANGE);
}
END_TEST

START_TEST(test_overflow) {
  unsigned short port = -1;
  ck_assert_ptr_eq(portToUri(port), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EPORTRANGE);
}
END_TEST

TCase* test_case_portToUri() {
  TCase* tc = tcase_create("portToUri");
  tcase_add_test(tc, test_0);
  tcase_add_test(tc, test_4242);
  tcase_add_test(tc, test_overflow);
  return tc;
}
