#include "tc_codeStateFromURI.h"

#include "utils/oidc_error.h"
#include "utils/uriUtils.h"

START_TEST(test_NULL) {
  struct codeState codeState = codeStateFromURI(NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
  ck_assert_ptr_eq(codeState.code, NULL);
  ck_assert_ptr_eq(codeState.state, NULL);
  ck_assert_ptr_eq(codeState.uri, NULL);
}
END_TEST

START_TEST(test_Empty) {
  struct codeState codeState = codeStateFromURI("");
  ck_assert_ptr_eq(codeState.code, NULL);
  ck_assert_ptr_eq(codeState.state, NULL);
  ck_assert_ptr_eq(codeState.uri, NULL);
  ck_assert_int_eq(oidc_errno, OIDC_ENOBASEURI);
}
END_TEST

START_TEST(test_noParam) {
  struct codeState codeState = codeStateFromURI("http://localhost:4242/");
  ck_assert_ptr_eq(codeState.code, NULL);
  ck_assert_ptr_eq(codeState.state, NULL);
  ck_assert_str_eq(codeState.uri, "http://localhost:4242/");
  ck_assert_int_eq(oidc_errno, OIDC_ENOSTATE);
}
END_TEST

START_TEST(test_noState) {
  struct codeState codeState =
      codeStateFromURI("http://localhost:4242/?code=1234");
  ck_assert_ptr_eq(codeState.state, NULL);
  ck_assert_str_eq(codeState.code, "1234");
  ck_assert_str_eq(codeState.uri, "http://localhost:4242/");
  ck_assert_int_eq(oidc_errno, OIDC_ENOSTATE);
}
END_TEST

START_TEST(test_noCode) {
  struct codeState codeState =
      codeStateFromURI("http://localhost:4242/?state=1234");
  ck_assert_ptr_eq(codeState.code, NULL);
  ck_assert_str_eq(codeState.state, "1234");
  ck_assert_str_eq(codeState.uri, "http://localhost:4242/");
  ck_assert_int_eq(oidc_errno, OIDC_ENOCODE);
}
END_TEST

START_TEST(test_statecode) {
  struct codeState codeState =
      codeStateFromURI("http://localhost:4242/?state=1234&code=asdf");
  ck_assert_str_eq(codeState.code, "asdf");
  ck_assert_str_eq(codeState.state, "1234");
  ck_assert_str_eq(codeState.uri, "http://localhost:4242/");
}
END_TEST

START_TEST(test_codestate) {
  struct codeState codeState =
      codeStateFromURI("http://localhost:4242/?code=asdf&state=1234");
  ck_assert_str_eq(codeState.code, "asdf");
  ck_assert_str_eq(codeState.state, "1234");
  ck_assert_str_eq(codeState.uri, "http://localhost:4242/");
}
END_TEST

TCase* test_case_codeStateFromURI() {
  TCase* tc = tcase_create("codeStateFromURI");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_Empty);
  tcase_add_test(tc, test_noParam);
  tcase_add_test(tc, test_noCode);
  tcase_add_test(tc, test_noState);
  tcase_add_test(tc, test_statecode);
  tcase_add_test(tc, test_codestate);
  return tc;
}
