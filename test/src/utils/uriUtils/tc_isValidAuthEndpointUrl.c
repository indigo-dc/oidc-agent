#include "tc_isValidAuthEndpointUrl.h"

#include "utils/uriUtils.h"

START_TEST(test_validHttps) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/authorize"), 1);
}
END_TEST

START_TEST(test_validHttp) {
  ck_assert_int_eq(isValidAuthEndpointUrl("http://idp.example.com/authorize"),
                   1);
}
END_TEST

START_TEST(test_validLocalhost) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("http://localhost:8080/authorize"), 1);
}
END_TEST

START_TEST(test_validSubDelims) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl(
          "https://idp.example.com/auth?x=$&y=(z)&a=b;c,d"), 1);
}
END_TEST

START_TEST(test_validPercentEncoding) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/auth?x=%20%2F"), 1);
}
END_TEST

START_TEST(test_validHttpsOnlyScheme) {
  ck_assert_int_eq(isValidAuthEndpointUrl("https://"), 0);
}
END_TEST

START_TEST(test_validHttpOnlyScheme) {
  ck_assert_int_eq(isValidAuthEndpointUrl("http://"), 0);
}
END_TEST

START_TEST(test_uriNULL) {
  ck_assert_int_eq(isValidAuthEndpointUrl(NULL), 0);
}
END_TEST

START_TEST(test_emptyUri) {
  ck_assert_int_eq(isValidAuthEndpointUrl(""), 0);
}
END_TEST

START_TEST(test_wrongSchemeFtp) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("ftp://idp.example.com/authorize"), 0);
}
END_TEST

START_TEST(test_wrongSchemeJavascript) {
  ck_assert_int_eq(isValidAuthEndpointUrl("javascript:alert(1)"), 0);
}
END_TEST

START_TEST(test_wrongSchemeFile) {
  ck_assert_int_eq(isValidAuthEndpointUrl("file:///etc/passwd"), 0);
}
END_TEST

START_TEST(test_containsSpace) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/a uthorize"), 0);
}
END_TEST

START_TEST(test_containsTab) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/\tauthorize"), 0);
}
END_TEST

START_TEST(test_containsNewline) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/\nauthorize"), 0);
}
END_TEST

START_TEST(test_containsControlChar) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/\x01authorize"), 0);
}
END_TEST

START_TEST(test_containsLessThan) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/<authorize"), 0);
}
END_TEST

START_TEST(test_containsGreaterThan) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/>authorize"), 0);
}
END_TEST

START_TEST(test_containsBacktick) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/`authorize"), 0);
}
END_TEST

START_TEST(test_containsPipe) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/|authorize"), 0);
}
END_TEST

START_TEST(test_containsDoubleQuote) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl("https://idp.example.com/\"authorize"), 0);
}
END_TEST

START_TEST(test_pocPayload) {
  ck_assert_int_eq(
      isValidAuthEndpointUrl(
          "http://127.0.0.1:18080/auth/$(id>/tmp/oidc-agent-cmdinj-proof)"),
      0);
}
END_TEST

TCase* test_case_isValidAuthEndpointUrl() {
  TCase* tc = tcase_create("isValidAuthEndpointUrl");
  tcase_add_test(tc, test_validHttps);
  tcase_add_test(tc, test_validHttp);
  tcase_add_test(tc, test_validLocalhost);
  tcase_add_test(tc, test_validSubDelims);
  tcase_add_test(tc, test_validPercentEncoding);
  tcase_add_test(tc, test_validHttpsOnlyScheme);
  tcase_add_test(tc, test_validHttpOnlyScheme);
  tcase_add_test(tc, test_uriNULL);
  tcase_add_test(tc, test_emptyUri);
  tcase_add_test(tc, test_wrongSchemeFtp);
  tcase_add_test(tc, test_wrongSchemeJavascript);
  tcase_add_test(tc, test_wrongSchemeFile);
  tcase_add_test(tc, test_containsSpace);
  tcase_add_test(tc, test_containsTab);
  tcase_add_test(tc, test_containsNewline);
  tcase_add_test(tc, test_containsControlChar);
  tcase_add_test(tc, test_containsLessThan);
  tcase_add_test(tc, test_containsGreaterThan);
  tcase_add_test(tc, test_containsBacktick);
  tcase_add_test(tc, test_containsPipe);
  tcase_add_test(tc, test_containsDoubleQuote);
  tcase_add_test(tc, test_pocPayload);
  return tc;
}
