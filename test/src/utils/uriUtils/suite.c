#include "suite.h"
#include "tc_codeStateFromURI.h"
#include "tc_extractParameterValueFromUri.h"

Suite* test_suite_uriUtils() {
  Suite* ts_uriUtils = suite_create("uriUtils");
  suite_add_tcase(ts_uriUtils, test_case_codeStateFromURI());
  suite_add_tcase(ts_uriUtils, test_case_extractParameterValueFromUri());
  return ts_uriUtils;
}
