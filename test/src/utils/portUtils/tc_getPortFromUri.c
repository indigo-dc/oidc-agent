#include "tc_getPortFromUri.h"

#include "utils/oidc_error.h"
#include "utils/portUtils.h"

START_TEST(test_4242s) {
  ck_assert_int_eq(getPortFromUri("http://localhost:4242/"), 4242);
}
END_TEST

START_TEST(test_4242) {
  ck_assert_int_eq(getPortFromUri("http://localhost:4242"), 4242);
}
END_TEST

START_TEST(test_4242path) {
  ck_assert_int_eq(getPortFromUri("http://localhost:4242/redirect"), 4242);
}
END_TEST

START_TEST(test_empty) {
  ck_assert_int_eq(getPortFromUri(""), 0);
  ck_assert_int_eq(oidc_errno, OIDC_EFMT);
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_int_eq(getPortFromUri(NULL), 0);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_noPort) {
  ck_assert_int_eq(getPortFromUri("http://localhost/"), 0);
  ck_assert_int_eq(oidc_errno, OIDC_EFMT);
}
END_TEST

START_TEST(test_noPort2) {
  ck_assert_int_eq(getPortFromUri("http://localhost:noPort/"), 0);
  ck_assert_int_eq(oidc_errno, OIDC_EFMT);
}
END_TEST

TCase* test_case_getPortFromUri() {
  TCase* tc = tcase_create("getPortFromUri");
  tcase_add_test(tc, test_4242);
  tcase_add_test(tc, test_4242s);
  tcase_add_test(tc, test_4242path);
  tcase_add_test(tc, test_empty);
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_noPort);
  tcase_add_test(tc, test_noPort2);
  return tc;
}
