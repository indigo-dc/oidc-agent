#include "fileUtils.h"

#include "oidc_file_io.h"
#include "utils/crypt.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <stdlib.h>

/**
 * @brief asserts that the oidc directory exists
 */
void assertOidcDirExists() {
  char* dir = NULL;
  if ((dir = getOidcDir()) == NULL) {
    printError("Error: oidc-dir does not exist. Run make to create it.\n");
    exit(EXIT_FAILURE);
  }
  secFree(dir);
}
