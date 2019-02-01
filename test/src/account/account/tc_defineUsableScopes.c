#include "tc_defineUsableScopes.h"

#include "account/account.h"
#include "defines/oidc_values.h"
#include "list/list.h"

extern list_t* defineUsableScopeList(const struct oidc_account* account);
extern void    _printList(list_t* l);

START_TEST(test_bothNull) {
  struct oidc_account account = {};
  account_setScope(&account, NULL);
  account_setScopesSupported(&account, NULL);
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
}
END_TEST

START_TEST(test_bothNullGoogle) {
  struct oidc_account account = {};
  account_setScope(&account, NULL);
  account_setScopesSupported(&account, NULL);
  account_setIssuerUrl(&account, GOOGLE_ISSUER_URL);
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_eq(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
}
END_TEST

START_TEST(test_supportedNull) {
  struct oidc_account account = {};
  account_setScope(&account,
                   oidc_strcopy("openid profile email offline_access"));
  account_setScopesSupported(&account, NULL);
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
  ck_assert_ptr_ne(list_find(list, "profile"), NULL);
  ck_assert_ptr_ne(list_find(list, "email"), NULL);
}
END_TEST

START_TEST(test_wantedNull) {
  struct oidc_account account = {};
  account_setScope(&account, NULL);
  account_setScopesSupported(
      &account, oidc_strcopy("openid profile email offline_access"));
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
  ck_assert_ptr_eq(list_find(list, "profile"), NULL);
  ck_assert_ptr_eq(list_find(list, "email"), NULL);
}
END_TEST

START_TEST(test_supportedNullGoogle) {
  struct oidc_account account = {};
  account_setScope(&account,
                   oidc_strcopy("openid profile email offline_access"));
  account_setScopesSupported(&account, NULL);
  account_setIssuerUrl(&account, GOOGLE_ISSUER_URL);
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_eq(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
  ck_assert_ptr_ne(list_find(list, "profile"), NULL);
  ck_assert_ptr_ne(list_find(list, "email"), NULL);
}
END_TEST

START_TEST(test_wantedNullGoogle) {
  struct oidc_account account = {};
  account_setScope(&account, NULL);
  account_setScopesSupported(&account, oidc_strcopy("openid profile email"));
  account_setIssuerUrl(&account, GOOGLE_ISSUER_URL);
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_eq(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
  ck_assert_ptr_eq(list_find(list, "profile"), NULL);
  ck_assert_ptr_eq(list_find(list, "email"), NULL);
}
END_TEST

START_TEST(test_bothValid) {
  struct oidc_account account = {};
  account_setScope(&account,
                   oidc_strcopy("profile email offline_access handy"));
  account_setScopesSupported(&account,
                             oidc_strcopy("openid profile email address"));
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
  ck_assert_ptr_ne(list_find(list, "profile"), NULL);
  ck_assert_ptr_ne(list_find(list, "email"), NULL);
}
END_TEST

START_TEST(test_max) {
  struct oidc_account account = {};
  account_setScope(&account, oidc_strcopy("max"));
  account_setScopesSupported(
      &account, oidc_strcopy("openid profile email address offline_access"));
  list_t* list = defineUsableScopeList(&account);
  // _printList(list);
  ck_assert_ptr_ne(list, NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OPENID), NULL);
  ck_assert_ptr_ne(list_find(list, OIDC_SCOPE_OFFLINE_ACCESS), NULL);
  ck_assert_ptr_ne(list_find(list, "profile"), NULL);
  ck_assert_ptr_ne(list_find(list, "email"), NULL);
  ck_assert_ptr_ne(list_find(list, "address"), NULL);
  ck_assert_ptr_eq(list_find(list, "max"), NULL);
}
END_TEST

TCase* test_case_defineUsableScopes() {
  TCase* tc = tcase_create("defineUsableScopes");
  tcase_add_test(tc, test_bothNullGoogle);
  tcase_add_test(tc, test_bothNull);
  tcase_add_test(tc, test_supportedNull);
  tcase_add_test(tc, test_wantedNull);
  tcase_add_test(tc, test_supportedNullGoogle);
  tcase_add_test(tc, test_wantedNullGoogle);
  tcase_add_test(tc, test_bothValid);
  tcase_add_test(tc, test_max);
  // TODO
  return tc;
}
