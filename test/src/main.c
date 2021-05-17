#include "test/src/account/account/suite.h"
#include "test/src/utils/crypt/crypt/suite.h"
#include "test/src/utils/crypt/memoryCrypt/suite.h"
#include "test/src/utils/json/suite.h"
#include "test/src/utils/portUtils/suite.h"
#include "test/src/utils/stringUtils/suite.h"
#include "test/src/utils/uriUtils/suite.h"

#include <check.h>
#include <stdlib.h>
#include <syslog.h>

int runSuite(Suite* suite) {
  int      number_failed;
  SRunner* sr = srunner_create(suite);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return number_failed;
}

int main() {
  setlogmask(LOG_UPTO(LOG_ERR));
  int number_failed = 0;
  number_failed |= runSuite(test_suite_json());
  number_failed |= runSuite(test_suite_portUtils());
  number_failed |= runSuite(test_suite_stringUtils());
  number_failed |= runSuite(test_suite_memoryCrypt());
  number_failed |= runSuite(test_suite_crypt());
  number_failed |= runSuite(test_suite_account());
  number_failed |= runSuite(test_suite_uriUtils());
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
