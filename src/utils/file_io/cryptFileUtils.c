#include "cryptFileUtils.h"

#include "utils/crypt/cryptUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "wrapper/list.h"

/**
 * @brief encrypts and writes a given text with the given password.
 * @param text the text to be encrypted
 * @param filepath an absolute path to the output file
 * @param password the encryption password
 * @return an oidc_error code. oidc_errno is set properly.
 */
oidc_error_t encryptAndWriteToFile(const char* text, const char* filepath,
                                   const char* password) {
  if (text == NULL || password == NULL || filepath == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* toWrite = encryptWithVersionLine(text, password);
  if (toWrite == NULL) {
    return oidc_errno;
  }
  logger(DEBUG, "Write to file %s", filepath);
  writeFile(filepath, toWrite);
  secFree(toWrite);
  return OIDC_SUCCESS;
}

oidc_error_t encryptAndWriteToOidcFile(const char* text, const char* filename,
                                       const char* password) {
  if (text == NULL || password == NULL || filename == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  logger(DEBUG, "Write to oidc file %s", filename);
  char*        filepath = concatToOidcDir(filename);
  oidc_error_t ret      = encryptAndWriteToFile(text, filepath, password);
  secFree(filepath);
  return ret;
}

/**
 * @brief decrypts a file in the oidcdir with the given password
 * @param filename the filename of the oidc-file
 * @param password if not @c NULL @p password is used for decryption; if @c NULL
 * the user will be prompted
 * @return a pointer to the decrypted filecontent. It has to be freed after
 * usage.
 */
char* decryptOidcFile(const char* filename, const char* password) {
  if (filename == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* filepath = concatToOidcDir(filename);
  char* ret      = decryptFile(filepath, password);
  secFree(filepath);
  return ret;
}

char* decryptFile(const char* filepath, const char* password) {
  if (filepath == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (!fileDoesExist(filepath)) {
    return NULL;
  }
  list_t* lines = getLinesFromFile(filepath);
  if (lines == NULL) {
    return NULL;
  }
  char* ret = decryptLinesList(lines, password);
  secFreeList(lines);
  return ret;
}
