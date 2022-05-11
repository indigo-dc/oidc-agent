#include "tc_createFinalTemplate.h"

#include <stdio.h>

#include "utils/json.h"
#include "utils/mytoken/mytokenUtils.h"
#include "wrapper/cjson.h"

#define data                                     \
  "{"                                            \
  "    \"f_1\": {"                               \
  "        \"key\": \"value\","                  \
  "        \"include\": \"f_2\""                 \
  "    },"                                       \
  "    \"f_2\": [{"                              \
  "            \"other\": 2,"                    \
  "            \"include\": [\"f_3\", \"!f_4\"]" \
  "        },"                                   \
  "        {"                                    \
  "            \"second_entry\": true,"          \
  "            \"include\": \"f_4\""             \
  "        }"                                    \
  "    ],"                                       \
  "    \"f_3\": {"                               \
  "        \"else\": 3,"                         \
  "        \"key\": \"other\""                   \
  "    },"                                       \
  "    \"f_4\": {"                               \
  "        \"else\": \"ok\","                    \
  "        \"hello\": true"                      \
  "    }"                                        \
  "}"

cJSON* dummyData = NULL;
void   initData() { dummyData = stringToJson(data); }

cJSON* readDummyData(const char* name) {
  if (dummyData == NULL) {
    initData();
  }
  return cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(dummyData, name), 1);
}

START_TEST(test_A) {
  cJSON* in  = stringToJson("\"!f_1\"");
  cJSON* res = createFinalTemplate(in, readDummyData);
  secFreeJson(in);
  ck_assert_ptr_ne(res, NULL);
  char* json = cJSON_PrintUnformatted(res);
  secFreeJson(res);
  ck_assert_str_eq(
      "[{\"key\":\"value\",\"include\":\"f_2\"},{\"hello\":true,\"else\":3,"
      "\"key\":\"other\",\"other\":2,\"include\":[\"f_3\",\"!f_4\"]},{\"else\":"
      "\"ok\",\"hello\":true,\"second_entry\":true,\"include\":\"f_4\"}]",
      json);
  secFree(json);
}
END_TEST

TCase* test_case_createFinalTemplate() {
  TCase* tc = tcase_create("createFinalTemplate");
  tcase_add_test(tc, test_A);
  return tc;
}
