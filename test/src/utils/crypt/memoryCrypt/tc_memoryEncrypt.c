#include "tc_memoryEncrypt.h"

#include "utils/crypt/memoryCrypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

START_TEST(test_NULL) {
  ck_assert_ptr_eq(memoryEncrypt(NULL), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_encrypt) {
  char* cipher = memoryEncrypt("test");
  ck_assert_ptr_ne(cipher, NULL);
  ck_assert_str_ne(cipher, "test");
  secFree(cipher);
}
END_TEST

TCase* test_case_memoryEncrypt() {
  TCase* tc = tcase_create("memoryEncrypt");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_encrypt);
  return tc;
}
