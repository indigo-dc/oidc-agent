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

unsigned char* decryptFileContent(const char* fileContent,
                                  const char* password) {
  if (fileContent == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int   len      = strlen(fileContent);
  char* fileText = secAlloc(sizeof(char) * (len + 1));
  strcpy(fileText, fileContent);
  unsigned long  cipher_len = atoi(strtok(fileText, ":"));
  char*          salt_hex   = strtok(NULL, ":");
  char*          nonce_hex  = strtok(NULL, ":");
  char*          cipher     = strtok(NULL, ":");
  unsigned char* decrypted =
      crypt_decrypt(cipher, cipher_len, password, nonce_hex, salt_hex);
  secFree(fileText);
  return decrypted;
}
