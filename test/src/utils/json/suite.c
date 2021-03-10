#include "suite.h"
#include "tc_isJSONObject.h"
#include "tc_setJSONValue.h"

Suite* test_suite_json() {
  Suite* ts_json = suite_create("json");
  suite_add_tcase(ts_json, test_case_isJSONObject());
  suite_add_tcase(ts_json, test_case_setJSONValue());
  return ts_json;
}
