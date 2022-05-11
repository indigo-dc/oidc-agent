#include "suite.h"

#include "tc_createFinalTemplate.h"

Suite* test_suite_mytokenUtils() {
  Suite* ts_portUtils = suite_create("mytokenUtils");
  suite_add_tcase(ts_portUtils, test_case_createFinalTemplate());
  return ts_portUtils;
}
