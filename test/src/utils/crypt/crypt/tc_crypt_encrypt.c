#include "tc_crypt_encrypt.h"

#include "utils/crypt/crypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

START_TEST(test_NULL) {
  char* buffer = "buffer";
  ck_assert_ptr_eq(crypt_encrypt(NULL, buffer), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
  ck_assert_ptr_eq(crypt_encrypt(buffer, NULL), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_encrypt) {
  char* cipher = crypt_encrypt("test", "password");
  ck_assert_ptr_ne(cipher, NULL);
  ck_assert_str_ne(cipher, "test");
  secFree(cipher);
}
END_TEST

TCase* test_case_crypt_encrypt() {
  TCase* tc = tcase_create("crypt_encrypt");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_encrypt);
  return tc;
}
