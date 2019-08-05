#include "suite.h"
#include "tc_cjose_getEncryptHeader.h"
#include "tc_cjose_getSignHeader.h"
#include "tc_createRSAKeys.h"
#include "tc_jose_encrypt.h"
#include "tc_jose_sign.h"
#include "tc_jose_signAndEncrypt.h"

Suite* test_suite_joseUtils() {
  Suite* ts_joseUtils = suite_create("joseUtils");
  // suite_add_tcase(ts_joseUtils, test_case_cjose_getEncryptHeader());
  // suite_add_tcase(ts_joseUtils, test_case_cjose_getSignHeader());
  // suite_add_tcase(ts_joseUtils, test_case_createRSAKeys());
  // suite_add_tcase(ts_joseUtils, test_case_jose_encrypt());
  suite_add_tcase(ts_joseUtils, test_case_jose_sign());
  // suite_add_tcase(ts_joseUtils, test_case_jose_signAndEncrypt());

  return ts_joseUtils;
}
