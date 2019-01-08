#include "tc_findRedirectUriByPort.h"

#include "utils/oidc_error.h"
#include "utils/portUtils.h"

START_TEST(test_NULL) {
  ck_assert_msg(findRedirectUriByPort(NULL, 0) == NULL, "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_nested_NULL) {
  struct oidc_account a = {};
  ck_assert_msg(findRedirectUriByPort(&a, 0) == NULL, "wrong return value");
  ck_assert_msg(oidc_errno == OIDC_EARGNULLFUNC,
                "oidc_errno not correctly set");
}
END_TEST

START_TEST(test_oneUri) {
  struct oidc_account a    = {};
  list_t*             list = list_new();
  list_rpush(list, list_node_new("http://localhost:4242"));
  account_setRedirectUris(&a, list);
  ck_assert_msg(
      strcmp(findRedirectUriByPort(&a, 4242), "http://localhost:4242") == 0,
      "wrong redirect_uri returned");
  ck_assert_msg(findRedirectUriByPort(&a, 8080) == NULL,
                "falsly returned a redirect uri");
  account_setRedirectUris(&a, NULL);
}
END_TEST

START_TEST(test_multipleUri) {
  struct oidc_account a    = {};
  list_t*             list = list_new();
  list_rpush(list, list_node_new("http://localhost:4242"));
  list_rpush(list, list_node_new("http://localhost:8080"));
  account_setRedirectUris(&a, list);
  ck_assert_msg(
      strcmp(findRedirectUriByPort(&a, 4242), "http://localhost:4242") == 0,
      "wrong redirect_uri returned");
  ck_assert_msg(
      strcmp(findRedirectUriByPort(&a, 8080), "http://localhost:8080") == 0,
      "wrong redirect_uri returned");
  ck_assert_msg(findRedirectUriByPort(&a, 8090) == NULL,
                "falsly returned a redirect uri");
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
