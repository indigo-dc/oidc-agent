#include "tc_portToUri.h"

#include "utils/portUtils.h"

START_TEST(test_4242) {
  ck_assert_int_eq(strcmp(portToUri(4242), "http://localhost:4242"), 0);
}
END_TEST

START_TEST(test_0) {
  ck_assert_int_eq(strcmp(portToUri(0), "http://localhost:0"), 0);
}
END_TEST

START_TEST(test_overflow) {
  unsigned short    port = -1;
  const char* const fmt  = "http://localhost:%hu";
  char* str = calloc(sizeof(char), snprintf(NULL, 0, fmt, port) + 1);
  sprintf(str, fmt, port);
  ck_assert_int_eq(strcmp(portToUri(-1), str), 0);
  free(str);
}
END_TEST

TCase* test_case_portToUri() {
  TCase* tc = tcase_create("portToUri");
  tcase_add_test(tc, test_0);
  tcase_add_test(tc, test_4242);
  tcase_add_test(tc, test_overflow);
  return tc;
}
