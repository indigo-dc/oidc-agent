#include "tc_defineUsableScopes.h"

#include "account/account.h"
#include "defines/oidc_values.h"
#include "utils/string/stringUtils.h"
#include "wrapper/list.h"

extern list_t* defineUsableScopeList(const struct oidc_account* account);
extern void    _printList(list_t* l);

START_TEST(test_null) {
  struct oidc_account account = {};
  account_setAuthScope(&account, NULL);
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
}
END_TEST

START_TEST(test_nullGoogle) {
  struct oidc_account account = {};
  account_setAuthScope(&account, NULL);
  account_setIssuerUrl(&account, GOOGLE_ISSUER_URL);
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_eq(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
}
END_TEST

START_TEST(test_valid) {
  struct oidc_account account = {};
  account_setAuthScope(&account,
                       oidc_strcopy("profile email offline_access handy"));
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
  ck_assert_ptr_ne(list_find(list, "profile"), NULL);
  ck_assert_ptr_ne(list_find(list, "email"), NULL);
  ck_assert_ptr_ne(list_find(list, "handy"), NULL);
}
END_TEST

TCase* test_case_defineUsableScopes() {
  TCase* tc = tcase_create("defineUsableScopes");
  tcase_add_test(tc, test_nullGoogle);
  tcase_add_test(tc, test_null);
  tcase_add_test(tc, test_valid);
  return tc;
}
