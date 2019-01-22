#include "suite.h"
#include "tc_findRedirectUriByPort.h"
#include "tc_getPortFromUri.h"
#include "tc_portToUri.h"

Suite* test_suite_portUtils() {
  Suite* ts_portUtils = suite_create("portUtils");
  suite_add_tcase(ts_portUtils, test_case_portToUri());
  suite_add_tcase(ts_portUtils, test_case_getPortFromUri());
  suite_add_tcase(ts_portUtils, test_case_findRedirectUriByPort());
  return ts_portUtils;
}
