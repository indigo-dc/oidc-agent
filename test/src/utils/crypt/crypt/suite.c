#include "suite.h"

#include "tc_crypt_decrypt.h"
#include "tc_crypt_encrypt.h"
#include "tc_fromBase64.h"
#include "tc_fromBase64UrlSafe.h"
#include "tc_s256.h"
#include "tc_toBase64.h"
#include "tc_toBase64UrlSafe.h"

Suite* test_suite_crypt() {
  Suite* ts_crypt = suite_create("crypt");
  suite_add_tcase(ts_crypt, test_case_crypt_decrypt());
  suite_add_tcase(ts_crypt, test_case_crypt_encrypt());
  suite_add_tcase(ts_crypt, test_case_fromBase64());
  suite_add_tcase(ts_crypt, test_case_fromBase64UrlSafe());
  suite_add_tcase(ts_crypt, test_case_s256());
  suite_add_tcase(ts_crypt, test_case_toBase64());
  suite_add_tcase(ts_crypt, test_case_toBase64UrlSafe());

  return ts_crypt;
}
