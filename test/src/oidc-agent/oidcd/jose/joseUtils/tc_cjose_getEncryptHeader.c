#include "tc_cjose_getEncryptHeader.h"

#include "oidc-agent/oidcd/jose/joseUtils.h"

START_TEST(test_nullnull) {
  ck_assert_ptr_eq(cjose_getEncryptHeader(NULL, NULL), NULL);
}
END_TEST

START_TEST(test_oknull) {
  ck_assert_ptr_eq(cjose_getEncryptHeader("RSA-OAEP", NULL), NULL);
}
END_TEST

START_TEST(test_nullok) {
  ck_assert_ptr_eq(cjose_getEncryptHeader(NULL, "A256GCM"), NULL);
}
END_TEST

START_TEST(test_ok) {
  ck_assert_ptr_ne(cjose_getEncryptHeader("RSA-OAEP", "A256GCM"), NULL);
}
END_TEST

TCase* test_case_cjose_getEncryptHeader() {
  TCase* tc = tcase_create("cjose_getEncryptHeader");
  tcase_add_test(tc, test_nullnull);
  tcase_add_test(tc, test_nullok);
  tcase_add_test(tc, test_oknull);
  tcase_add_test(tc, test_ok);
  return tc;
}
