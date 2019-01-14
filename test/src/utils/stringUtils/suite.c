#include "suite.h"
#include "tc_escapeCharInStr.h"
#include "tc_getDateString.h"
#include "tc_oidc_sprintf.h"
#include "tc_oidc_strcat.h"
#include "tc_oidc_strcopy.h"
#include "tc_oidc_strncopy.h"
#include "tc_strCountChar.h"
#include "tc_strEnds.h"
#include "tc_strSubStringCase.h"
#include "tc_strToInt.h"
#include "tc_strToULong.h"
#include "tc_strValid.h"
#include "tc_strcaseequal.h"
#include "tc_strelim.h"
#include "tc_strelimIfAfter.h"
#include "tc_strelimIfFollowed.h"
#include "tc_strequal.h"
#include "tc_strstarts.h"

Suite* test_suite_stringUtils() {
  Suite* ts_stringUtils = suite_create("stringUtils");
  suite_add_tcase(ts_stringUtils, test_case_escapeCharInStr());
  suite_add_tcase(ts_stringUtils, test_case_getDateString());
  suite_add_tcase(ts_stringUtils, test_case_oidc_sprintf());
  suite_add_tcase(ts_stringUtils, test_case_oidc_strcat());
  suite_add_tcase(ts_stringUtils, test_case_oidc_strcopy());
  suite_add_tcase(ts_stringUtils, test_case_oidc_strncopy());
  suite_add_tcase(ts_stringUtils, test_case_strCountChar());
  suite_add_tcase(ts_stringUtils, test_case_strEnds());
  suite_add_tcase(ts_stringUtils, test_case_strSubStringCase());
  suite_add_tcase(ts_stringUtils, test_case_strToInt());
  suite_add_tcase(ts_stringUtils, test_case_strToULong());
  suite_add_tcase(ts_stringUtils, test_case_strValid());
  suite_add_tcase(ts_stringUtils, test_case_strcaseequal());
  suite_add_tcase(ts_stringUtils, test_case_strelim());
  suite_add_tcase(ts_stringUtils, test_case_strelimIfAfter());
  suite_add_tcase(ts_stringUtils, test_case_strelimIfFollowed());
  suite_add_tcase(ts_stringUtils, test_case_strequal());
  suite_add_tcase(ts_stringUtils, test_case_strstarts());

  return ts_stringUtils;
}
