#include "tc_s256.h"

#include "utils/crypt/crypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

START_TEST(test_NULL) {
  ck_assert_ptr_eq(s256(NULL), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_s256) {
  char* s = s256("test");
  ck_assert_ptr_ne(s, NULL);
  ck_assert_str_eq(s, "n4bQgYhMfWWaL-qgxVrQFaO_TxsrC4Is0V1sFbDwCgg");
  secFree(s);
}
END_TEST

TCase* test_case_s256() {
  TCase* tc = tcase_create("s256");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_s256);
  return tc;
}
