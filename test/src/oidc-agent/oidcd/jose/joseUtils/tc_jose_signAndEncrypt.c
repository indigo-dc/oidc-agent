#include "tc_jose_signAndEncrypt.h"

#include "oidc-agent/oidcd/jose/joseUtils.h"

// TODO
START_TEST(test_) { ck_assert(1 == 1); }
END_TEST

TCase* test_case_jose_signAndEncrypt() {
  TCase* tc = tcase_create("jose_signAndEncrypt");
  tcase_add_test(tc, test_);
  // TODO
  return tc;
}
