#include "tc_initMemoryCrypt.h"

#include "utils/crypt/memoryCrypt.h"
#include "utils/memory.h"

START_TEST(test_differ) {
  extern unsigned long memoryPass;
  initMemoryCrypt();
  unsigned long pass1 = memoryPass;
  initMemoryCrypt();
  unsigned long pass2 = memoryPass;
  ck_assert_uint_ne(pass1, pass2);
}
END_TEST

TCase* test_case_initMemoryCrypt() {
  TCase* tc = tcase_create("initMemoryCrypt");
  tcase_add_test(tc, test_differ);
  return tc;
}
