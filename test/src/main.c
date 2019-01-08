#include "test/src/utils/json/suite.h"
#include "test/src/utils/portUtils/suite.h"

#include <check.h>
#include <stdlib.h>

int runSuite(Suite* suite) {
  int      number_failed;
  SRunner* sr = srunner_create(suite);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return number_failed;
}

int main() {
  int number_failed = 0;
  number_failed |= runSuite(test_suite_json());
  number_failed |= runSuite(test_suite_portUtils());
  // TODO
  // number_failed |= runSuite(test_suite_fileio());
  // number_failed |= runSuite(test_suite_crypt());
  // number_failed |= runSuite(test_suite_strings());
  // number_failed |= runSuite(test_suite_list());
  // ...
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
