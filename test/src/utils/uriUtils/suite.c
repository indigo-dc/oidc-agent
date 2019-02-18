#include "suite.h"
#include "tc_codeStateFromURI.h"


Suite* test_suite_uriUtils() {
  Suite* ts_uriUtils = suite_create("uriUtils");
  	suite_add_tcase(ts_uriUtils, test_case_codeStateFromURI());

  return ts_uriUtils;
}
