#include "tc_memoryDecrypt.h"

#include "utils/crypt/memoryCrypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

START_TEST(test_NULL) {
  ck_assert_ptr_eq(memoryDecrypt(NULL), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_SUCCESS);
}
END_TEST

START_TEST(test_decrypt) {
  char* cipher = memoryEncrypt("test");
  char* plain  = memoryDecrypt(cipher);
  secFree(cipher);
  ck_assert_ptr_ne(plain, NULL);
  ck_assert_str_eq(plain, "test");
  secFree(plain);
}
END_TEST

TCase* test_case_memoryDecrypt() {
  TCase* tc = tcase_create("memoryDecrypt");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_decrypt);
  return tc;
}
