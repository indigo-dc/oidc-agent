#include "suite.h"
#include "tc_defineUsableScopes.h"

Suite* test_suite_account() {
  Suite* ts_account = suite_create("account");
  suite_add_tcase(ts_account, test_case_defineUsableScopes());
  return ts_account;
}
