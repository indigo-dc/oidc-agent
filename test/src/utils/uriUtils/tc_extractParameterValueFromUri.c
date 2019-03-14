#include "tc_extractParameterValueFromUri.h"

#include "utils/oidc_error.h"
#include "utils/uriUtils.h"

START_TEST(test_uriNULL) {
  char* param = extractParameterValueFromUri(NULL, "code");
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
  ck_assert_ptr_eq(param, NULL);
}
END_TEST

START_TEST(test_paramNULL) {
  char* param =
      extractParameterValueFromUri("http://localhost:4242/?code=1234", NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
  ck_assert_ptr_eq(param, NULL);
}
END_TEST

START_TEST(test_emptyUri) {
  char* param = extractParameterValueFromUri("", "code");
  ck_assert_ptr_eq(param, NULL);
}
END_TEST

START_TEST(test_firstParam) {
  char* param = extractParameterValueFromUri(
      "http://localhost:4242/?code=1234&other=5", "code");
  ck_assert_ptr_ne(param, NULL);
  ck_assert_str_eq(param, "1234");
}
END_TEST

START_TEST(test_lastParam) {
  char* param = extractParameterValueFromUri(
      "http://localhost:4242/?code=1234&other=5", "other");
  ck_assert_ptr_ne(param, NULL);
  ck_assert_str_eq(param, "5");
}
END_TEST

START_TEST(test_middleParam) {
  char* param = extractParameterValueFromUri(
      "http://localhost:4242/?code=1234&other=5&last=last", "other");
  ck_assert_ptr_ne(param, NULL);
  ck_assert_str_eq(param, "5");
}
END_TEST

START_TEST(test_onlyParam) {
  char* param =
      extractParameterValueFromUri("http://localhost:4242/?code=1234", "code");
  ck_assert_ptr_ne(param, NULL);
  ck_assert_str_eq(param, "1234");
}
END_TEST

START_TEST(test_notFound) {
  char* param =
      extractParameterValueFromUri("http://localhost:4242/?code=1234", "other");
  ck_assert_ptr_eq(param, NULL);
}
END_TEST

TCase* test_case_extractParameterValueFromUri() {
  TCase* tc = tcase_create("codeStateFromURI");
  tcase_add_test(tc, test_uriNULL);
  tcase_add_test(tc, test_paramNULL);
  tcase_add_test(tc, test_emptyUri);
  tcase_add_test(tc, test_firstParam);
  tcase_add_test(tc, test_lastParam);
  tcase_add_test(tc, test_middleParam);
  tcase_add_test(tc, test_onlyParam);
  tcase_add_test(tc, test_notFound);
  return tc;
}
