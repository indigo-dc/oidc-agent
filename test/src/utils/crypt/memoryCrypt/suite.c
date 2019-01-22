#include "suite.h"
#include "tc_initMemoryCrypt.h"
#include "tc_memoryDecrypt.h"
#include "tc_memoryEncrypt.h"

Suite* test_suite_memoryCrypt() {
  Suite* ts_memoryCrypt = suite_create("memoryCrypt");
  suite_add_tcase(ts_memoryCrypt, test_case_memoryDecrypt());
  suite_add_tcase(ts_memoryCrypt, test_case_memoryEncrypt());
  suite_add_tcase(ts_memoryCrypt, test_case_initMemoryCrypt());

  return ts_memoryCrypt;
}
