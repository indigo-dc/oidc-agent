#include "promptCryptFileUtils.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/memory.h"
#include "utils/promptUtils.h"

oidc_error_t _promptAndCryptAndWriteToAnyFile(
    const char* text, const char* filepath, const char* oidc_filename,
    const char* hint, const char* suggestedPassword, const char* pw_cmd,
    const char* pw_file, const char* pw_env) {
  if (text == NULL || hint == NULL ||
      (filepath == NULL && oidc_filename == NULL)) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  oidc_error_t (*encryptAndWriteFnc)(const char*, const char*, const char*) =
      encryptAndWriteToFile;
  if (oidc_filename != NULL) {
    encryptAndWriteFnc = encryptAndWriteToOidcFile;
  }
  char* encryptionPassword = getEncryptionPasswordFor(hint, suggestedPassword,
                                                      pw_cmd, pw_file, pw_env);
  if (encryptionPassword == NULL) {
    return oidc_errno;
  }
  oidc_error_t ret =
      encryptAndWriteFnc(text, oidc_filename ?: filepath, encryptionPassword);
  secFree(encryptionPassword);
  return ret;
}

oidc_error_t promptEncryptAndWriteToFile(const char* text, const char* filepath,
                                         const char* hint,
                                         const char* suggestedPassword,
                                         const char* pw_cmd,
                                         const char* pw_file,
                                         const char* pw_env) {
  if (text == NULL || filepath == NULL || hint == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  return _promptAndCryptAndWriteToAnyFile(
      text, filepath, NULL, hint, suggestedPassword, pw_cmd, pw_file, pw_env);
}

oidc_error_t promptEncryptAndWriteToOidcFile(
    const char* text, const char* filename, const char* hint,
    const char* suggestedPassword, const char* pw_cmd, const char* pw_file,
    const char* pw_env) {
  if (text == NULL || filename == NULL || hint == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  return _promptAndCryptAndWriteToAnyFile(
      text, NULL, filename, hint, suggestedPassword, pw_cmd, pw_file, pw_env);
}

struct resultWithEncryptionPassword getDecryptedFileAndPasswordFor(
    const char* filepath, const char* pw_cmd, const char* pw_file,
    const char* pw_env) {
  if (filepath == NULL) {
    oidc_setArgNullFuncError(__func__);
    return RESULT_WITH_PASSWORD_NULL;
  }
  if (!fileDoesExist(filepath)) {
    oidc_errno = OIDC_EFNEX;
    return RESULT_WITH_PASSWORD_NULL;
  }
  return _getDecryptedTextAndPasswordWithPromptFor(
      filepath, filepath, decryptFile, 0, pw_cmd, pw_file, pw_env);
}

struct resultWithEncryptionPassword getDecryptedOidcFileAndPasswordFor(
    const char* filename, const char* pw_cmd, const char* pw_file,
    const char* pw_env) {
  if (filename == NULL) {
    oidc_setArgNullFuncError(__func__);
    return RESULT_WITH_PASSWORD_NULL;
  }
  if (!oidcFileDoesExist(filename)) {
    oidc_errno = OIDC_EFNEX;
    return RESULT_WITH_PASSWORD_NULL;
  }
  return _getDecryptedTextAndPasswordWithPromptFor(
      filename, filename, decryptOidcFile, 1, pw_cmd, pw_file, pw_env);
}

char* getDecryptedFileFor(const char* filepath, const char* pw_cmd,
                          const char* pw_file, const char* pw_env) {
  if (filepath == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct resultWithEncryptionPassword res =
      getDecryptedFileAndPasswordFor(filepath, pw_cmd, pw_file, pw_env);
  secFree(res.password);
  return res.result;
}

char* getDecryptedOidcFileFor(const char* filename, const char* pw_cmd,
                              const char* pw_file, const char* pw_env) {
  if (filename == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct resultWithEncryptionPassword res =
      getDecryptedOidcFileAndPasswordFor(filename, pw_cmd, pw_file, pw_env);
  secFree(res.password);
  return res.result;
}
