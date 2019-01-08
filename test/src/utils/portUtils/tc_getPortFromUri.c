#include "tc_getPortFromUri.h"

#include "utils/oidc_error.h"
#include "utils/portUtils.h"

START_TEST(test_4242s) {
  ck_assert_msg(getPortFromUri("http://localhost:4242/") == 4242,
                "Getting port from uri with trailing slash fails");
}
END_TEST

START_TEST(test_4242) {
  ck_assert_msg(getPortFromUri("http://localhost:4242") == 4242,
                "Getting port from uri with no trailing slash fails");
}
END_TEST

START_TEST(test_4242path) {
  ck_assert_msg(getPortFromUri("http://localhost:4242/redirect") == 4242,
                "Getting port from uri with path fails");
}
END_TEST

START_TEST(test_empty) {
  ck_assert_msg(getPortFromUri("") == 0, "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EFMT, "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_NULL) {
  ck_assert_msg(getPortFromUri(NULL) == 0, "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_noPort) {
  ck_assert_msg(getPortFromUri("http://localhost/") == 0, "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EFMT, "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_noPort2) {
  ck_assert_msg(getPortFromUri("http://localhost:noPort/") == 0,
                "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EFMT, "oidc_errno not correctly set");
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
