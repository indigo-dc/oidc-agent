#include "tc_findRedirectUriByPort.h"

#include "utils/oidc_error.h"
#include "utils/portUtils.h"

START_TEST(test_NULL) {
  ck_assert_ptr_eq(findRedirectUriByPort(NULL, 0), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_nested_NULL) {
  struct oidc_account a = {};
  ck_assert_ptr_eq(findRedirectUriByPort(&a, 0), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_oneUri) {
  struct oidc_account a    = {};
  list_t*             list = list_new();
  list_rpush(list, list_node_new("http://localhost:4242"));
  account_setRedirectUris(&a, list);
  ck_assert_str_eq(findRedirectUriByPort(&a, 4242), "http://localhost:4242");
  ck_assert_ptr_eq(findRedirectUriByPort(&a, 8080), NULL);
  account_setRedirectUris(&a, NULL);
}
END_TEST

START_TEST(test_multipleUri) {
  struct oidc_account a    = {};
  list_t*             list = list_new();
  list_rpush(list, list_node_new("http://localhost:4242"));
  list_rpush(list, list_node_new("http://localhost:8080"));
  account_setRedirectUris(&a, list);
  ck_assert_str_eq(findRedirectUriByPort(&a, 4242), "http://localhost:4242");
  ck_assert_str_eq(findRedirectUriByPort(&a, 8080), "http://localhost:8080");
  ck_assert_ptr_eq(findRedirectUriByPort(&a, 8090), NULL);
  account_setRedirectUris(&a, NULL);
}
END_TEST

TCase* test_case_findRedirectUriByPort() {
  TCase* tc = tcase_create("findRedirectUriByPort");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_nested_NULL);
  tcase_add_test(tc, test_oneUri);
  tcase_add_test(tc, test_multipleUri);
  return tc;
}
