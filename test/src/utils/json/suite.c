#include "suite.h"
#include "tc_isJSONObject.h"
// TODO
// #include "tc_hasKey.h"
// #include "tc_mergeJSONObjects.h"

Suite* test_suite_json() {
  Suite* ts_json = suite_create("Json");
  suite_add_tcase(ts_json, test_case_isJSONObject());
  // TODO
  // suite_add_tcase(ts_json, test_case_hasKey());
  // suite_add_tcase(ts_json, test_case_mergeJSONObjects());
  // ...
  return ts_json;
}
