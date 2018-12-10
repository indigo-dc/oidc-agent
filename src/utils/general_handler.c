#include "general_handler.h"

// #ifdef INCLUDE_WRITE_HANDLERS

#include "settings.h"
#include "utils/crypt.h"
#include "utils/cryptUtils.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/oidc_error.h"
#include "utils/prompt.h"

#include <syslog.h>

/**
 * @brief encrypts and writes a given text.
 * @param text the json encoded account configuration text
 * @param suggestedPassword the suggestedPassword for encryption, won't be
 * displayed; can be NULL.
 * @param filepath an absolute path to the output file. Either filepath or
 * filename has to be given. The other one shall be NULL.
 * @param filename the filename of the output file. The output file will be
 * placed in the oidc dir. Either filepath or filename has to be given. The
 * other one shall be NULL.
 * @return an oidc_error code. oidc_errno is set properly.
 */
oidc_error_t encryptAndWriteText(const char* text, const char* hint,
                                 const char* suggestedPassword,
                                 const char* filepath,
                                 const char* oidc_filename) {
  initCrypt();
  char* encryptionPassword =
      getEncryptionPassword(hint, suggestedPassword, UINT_MAX);
  if (encryptionPassword == NULL) {
    return oidc_errno;
  }
  oidc_error_t ret = encryptAndWriteWithPassword(text, encryptionPassword,
                                                 filepath, oidc_filename);
  secFree(encryptionPassword);
  return ret;
}

char* getEncryptionPassword(const char* forWhat, const char* suggestedPassword,
                            unsigned int max_pass_tries) {
  char*        encryptionPassword = NULL;
  unsigned int i;
  unsigned int max_tries =
      max_pass_tries == 0 ? MAX_PASS_TRIES : max_pass_tries;
  for (i = 0; i < max_tries; i++) {
    char* input =
        promptPassword("Enter encryption password for %s%s: ", forWhat,
                       strValid(suggestedPassword) ? " [***]" : "");
    if (suggestedPassword &&
        !strValid(input)) {  // use same encryption password
      secFree(input);
      encryptionPassword = oidc_strcopy(suggestedPassword);
      return encryptionPassword;
    } else {
      encryptionPassword = input;
      char* confirm      = promptPassword("Confirm encryption Password: ");
      if (strcmp(encryptionPassword, confirm) != 0) {
        printError("Encryption passwords did not match.\n");
        secFree(confirm);
        secFree(encryptionPassword);
      } else {
        secFree(confirm);
        return encryptionPassword;
      }
    }
  }
  if (encryptionPassword) {
    secFree(encryptionPassword);
  }

  oidc_errno = OIDC_EMAXTRIES;
  return NULL;
}

/**
 * @brief encrypts and writes a given text with the given password.
 * @param text the text to be encrypted
 * @param password the encryption password
 * @param filepath an absolute path to the output file. Either filepath or
 * filename has to be given. The other one shall be NULL.
 * @param filename the filename of the output file. The output file will be
 * placed in the oidc dir. Either filepath or filename has to be given. The
 * other one shall be NULL.
 * @return an oidc_error code. oidc_errno is set properly.
 */
oidc_error_t encryptAndWriteWithPassword(const char* text, const char* password,
                                         const char* filepath,
                                         const char* oidc_filename) {
  if (text == NULL || password == NULL ||
      (filepath == NULL && oidc_filename == NULL)) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  char* toWrite = encryptWithVersionLine(text, password);
  if (toWrite == NULL) {
    return oidc_errno;
  }
  if (filepath) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Write to file %s", filepath);
    writeFile(filepath, toWrite);
  } else {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Write to oidc file %s", oidc_filename);
    writeOidcFile(oidc_filename, toWrite);
  }
  secFree(toWrite);
  return OIDC_SUCCESS;
}

// #endif  // INCLUDE_WRITE_HANDLERS
