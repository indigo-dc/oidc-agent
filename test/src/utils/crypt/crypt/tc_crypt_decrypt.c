#include "tc_crypt_decrypt.h"

#include "utils/crypt/crypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

START_TEST(test_NULL) {
  char* buffer = "buffer";
  ck_assert_ptr_eq(crypt_decrypt(NULL, buffer), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
  ck_assert_ptr_eq(crypt_encrypt(buffer, NULL), NULL);
  ck_assert_int_eq(oidc_errno, OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_decrypt) {
  char* plain =
      crypt_decrypt("20\nlzCHhZr0UL0or/"
                    "mk1hsoBU5euCQVLv5U\nCpq4aAHr+22bIH535xnKFQ==\n24:16:16:32:"
                    "1:2:67108864:2\n7vG6GHd45pLNqV5vdCGsAwat37o="
                    "\nD9MfcJjzEnXFD232v1MMbRQjHfkaJkifqz/uPPbLCXs=",
                    "password");
  ck_assert_ptr_ne(plain, NULL);
  ck_assert_str_eq(plain, "test");
  secFree(plain);
}
END_TEST

TCase* test_case_crypt_decrypt() {
  TCase* tc = tcase_create("crypt_decrypt");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_decrypt);
  return tc;
}
