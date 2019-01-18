#include "suite.h"
$INCLUDES$

Suite* test_suite_$NORMAL$() {
  Suite* ts_$NORMAL$ = suite_create("$NORMAL$");
  $ADDCASES$
  return ts_$NORMAL$;
}
