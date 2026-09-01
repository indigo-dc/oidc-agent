#include "suite.h"

#include "tc_codeStateFromURI.h"
#include "tc_extractParameterValueFromUri.h"
#include "tc_isValidAuthEndpointUrl.h"

Suite* test_suite_uriUtils() {
  Suite* ts_uriUtils = suite_create("uriUtils");
  suite_add_tcase(ts_uriUtils, test_case_codeStateFromURI());
  suite_add_tcase(ts_uriUtils, test_case_extractParameterValueFromUri());
  suite_add_tcase(ts_uriUtils, test_case_isValidAuthEndpointUrl());
  return ts_uriUtils;
}
