#include "fileUtils.h"

#include "../crypt.h"
#include "../file_io/oidc_file_io.h"
#include "memory.h"

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
