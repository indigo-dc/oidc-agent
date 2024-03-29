#include "tc_fromBase64UrlSafe.h"

#include "utils/crypt/crypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

START_TEST(test_NULL) {
  char* buffer = NULL;
  ck_assert_int_eq(fromBase64UrlSafe(NULL, 5, (unsigned char*)buffer),
                   OIDC_EARGNULLFUNC);
  ck_assert_int_eq(fromBase64UrlSafe(buffer, 5, NULL), OIDC_EARGNULLFUNC);
}
END_TEST

START_TEST(test_decode) {
  unsigned char s[5] = {0};
  int r = fromBase64UrlSafe("dGVzdA", 4, s);
  ck_assert_int_eq(r, 0);
  ck_assert_str_eq((char*)s, "test");
}
END_TEST

START_TEST(test_wrong_decode) {
  unsigned char s[5] = {0};
  int           r = fromBase64UrlSafe("dGVzdA=", 4, s);
  ck_assert_int_ne(r, 0);
}
END_TEST

TCase* test_case_fromBase64UrlSafe() {
  TCase* tc = tcase_create("fromBase64UrlSafe");
  tcase_add_test(tc, test_NULL);
  tcase_add_test(tc, test_decode);
  tcase_add_test(tc, test_wrong_decode);
  return tc;
}
