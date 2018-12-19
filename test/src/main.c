#include "utils/json.h"

#include <check.h>
#include <stdlib.h>

START_TEST(test_utils_json_checkJSONObjectEmpty) {
  const char* json = "{}";
  ck_assert_msg(isJSONObject(json) == 1, "Did not parse as json object");
}
END_TEST

START_TEST(test_utils_json_checkJSONObjectNoObject) {
  const char* json = "[]";
  ck_assert_msg(isJSONObject(json) == 0, "Did falsly parse as json object");
  json = "just a string";
  ck_assert_msg(isJSONObject(json) == 0, "Did falsly parse as json object");
}
END_TEST

START_TEST(test_utils_json_checkJSONObjectWithStringElement) {
  const char* json = "{\"key\":\"value\"}";
  ck_assert_msg(isJSONObject(json) == 1, "Did not parse as json object");
}
END_TEST

START_TEST(test_utils_json_checkJSONObjectWithNumberElement) {
  const char* json = "{\"key\":42}";
  ck_assert_msg(isJSONObject(json) == 1, "Did not parse as json object");
}
END_TEST

START_TEST(test_utils_json_checkJSONObjectWithArrayElement) {
  const char* json = "{\"key\":[10,20]}";
  ck_assert_msg(isJSONObject(json) == 1, "Did not parse as json object");
}
END_TEST

START_TEST(test_utils_json_checkJSONObjectWithObjectElement) {
  const char* json = "{\"key\":{\"innerkey\":42}}";
  ck_assert_msg(isJSONObject(json) == 1, "Did not parse as json object");
}
END_TEST

START_TEST(test_utils_json_checkJSONObjectWithMultipleElement) {
  const char* json =
      "{\"key\":\"value\",\"key2\":\"value2\", \"key3\" : \"value3\"\n}";
  ck_assert_msg(isJSONObject(json) == 1, "Did not parse as json object");
}
END_TEST

Suite* utils_suite() {
  Suite* ts_utils = suite_create("Utils");

  TCase* tc_json = tcase_create("Json");

  tcase_add_test(tc_json, test_utils_json_checkJSONObjectEmpty);
  tcase_add_test(tc_json, test_utils_json_checkJSONObjectNoObject);
  tcase_add_test(tc_json, test_utils_json_checkJSONObjectWithStringElement);
  tcase_add_test(tc_json, test_utils_json_checkJSONObjectWithNumberElement);
  tcase_add_test(tc_json, test_utils_json_checkJSONObjectWithArrayElement);
  tcase_add_test(tc_json, test_utils_json_checkJSONObjectWithObjectElement);
  tcase_add_test(tc_json, test_utils_json_checkJSONObjectWithMultipleElement);
  suite_add_tcase(ts_utils, tc_json);
  return ts_utils;
}

int main() {
  int      number_failed;
  Suite*   s  = utils_suite();
  SRunner* sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
