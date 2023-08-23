#include "promptUtils.h"

#include <stddef.h>

#include "defines/settings.h"
#include "getprompt.h"
#include "prompt.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/crypt/gpg/gpg.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"
#include "utils/system_runner.h"
#ifdef __MSYS__
#include "utils/registryConnector.h"
#endif

char* getEncryptionPasswordForAccountConfig(const char* shortname,
                                            const char* suggestedPassword,
                                            const char* pw_cmd,
                                            const char* pw_file,
                                            const char* pw_env) {
  char* forWhat = oidc_sprintf("account config '%s'", shortname);
  char* ret     = getEncryptionPasswordFor(forWhat, suggestedPassword, pw_cmd,
                                           pw_file, pw_env);
  secFree(forWhat);
  return ret;
}

char* getDecryptionPasswordForAccountConfig(
    const char* shortname, const char* pw_cmd, const char* pw_file,
    const char* pw_env, unsigned int max_pass_tries, unsigned int* number_try) {
  char* forWhat = oidc_sprintf("account config '%s'", shortname);
  char* ret     = getDecryptionPasswordFor(forWhat, pw_cmd, pw_file, pw_env,
                                           max_pass_tries, number_try);
  secFree(forWhat);
  return ret;
}

char* getEncryptionPasswordFor(const char* forWhat,
                               const char* suggestedPassword,
                               const char* pw_cmd, const char* pw_file,
                               const char* pw_env) {
  if (pw_env != NULL) {
#ifdef __MSYS__
    char* pass = getRegistryValue(pw_env);
#else
    char* pass = oidc_strcopy(getenv(pw_env));
#endif
    if (pass) {
      return pass;
    }
  }
  if (pw_cmd) {
    char* pass = getOutputFromCommand(pw_cmd);
    if (pass) {
      return pass;
    }
  }
  if (pw_file) {
    char* pass = getLineFromFile(pw_file);
    if (pass) {
      return pass;
    }
  }
  char*  encryptionPassword = NULL;
  cJSON* data        = generateJSONObject("shortname", cJSON_String, forWhat);
  char*  prompt_text = getprompt(PROMPTTEMPLATE(PASSWORD), data);
  secFreeJson(data);
  while (1) {
    char* input = promptPassword(prompt_text, "Encryption password",
                                 suggestedPassword, CLI_PROMPT_VERBOSE);
    if (strValid(suggestedPassword) && input &&
        strequal(suggestedPassword, input)) {  // use same encryption password
      secFree(input);
      encryptionPassword = oidc_strcopy(suggestedPassword);
      secFree(prompt_text);
      return encryptionPassword;
    } else {
      encryptionPassword  = input;
      char* prompt_text_2 = getprompt(PROMPTTEMPLATE(CONFIRM_PASSWORD), NULL);
      char* confirm = promptPassword(prompt_text_2, "Encryption password", NULL,
                                     CLI_PROMPT_VERBOSE);
      if (!strequal(encryptionPassword, confirm)) {
        printError("Encryption passwords did not match.\n");
        secFree(confirm);
        secFree(prompt_text_2);
        secFree(encryptionPassword);
      } else {
        secFree(confirm);
        secFree(prompt_text);
        return encryptionPassword;
      }
    }
  }
}

char* getDecryptionPasswordFor(const char* forWhat, const char* pw_cmd,
                               const char* pw_file, const char* pw_env,
                               unsigned int  max_pass_tries,
                               unsigned int* number_try) {
  unsigned int max_tries =
      max_pass_tries == 0 ? MAX_PASS_TRIES : max_pass_tries;
  if (pw_env && (number_try == NULL || *number_try == 0)) {
#ifndef __MSYS__
    char* pass = oidc_strcopy(getenv(pw_env));
#else
    char* pass = getRegistryValue(pw_env);
#endif
    if (pass) {
      if (number_try) {
        (*number_try)++;
      }
      return pass;
    }
  }
  if (pw_cmd && (number_try == NULL || *number_try == 0)) {
    char* pass = getOutputFromCommand(pw_cmd);
    if (pass) {
      if (number_try) {
        (*number_try)++;
      }
      return pass;
    }
  }
  if (pw_file && (number_try == NULL || *number_try == 0 ||
                  (*number_try == 1 && pw_cmd))) {
    char* pass = getLineFromFile(pw_file);
    if (pass) {
      if (number_try) {
        (*number_try)++;
      }
      return pass;
    }
  }
  if (number_try == NULL || *number_try < max_tries) {
    if (number_try) {
      (*number_try)++;
    }
    char* prompt_str = oidc_strcat("Enter decryption password for ", forWhat);
    char* input      = promptPassword(prompt_str, "Encryption password", NULL,
                                      CLI_PROMPT_VERBOSE);
    secFree(prompt_str);
    return input;
  }
  oidc_errno = OIDC_EMAXTRIES;
  return NULL;
}

struct resultWithEncryptionPassword _getDecryptedTextAndPasswordWithPromptFor(
    const char* file, unsigned char isAccountConfig, const char* pw_cmd,
    const char* pw_file, const char* pw_env) {
  if (file == NULL) {
    oidc_setArgNullFuncError(__func__);
    return RESULT_WITH_PASSWORD_NULL;
  }
  char* encrypted_content =
      isAccountConfig ? readOidcFile(file) : readFile(file);
  if (isPGPMessage(encrypted_content)) {
    char* plain = decryptPGPFileContent(encrypted_content);
    secFree(encrypted_content);
    return (struct resultWithEncryptionPassword){.result   = plain,
                                                 .password = NULL};
  }

  char* (*getPasswordFnc)(const char*, const char*, const char*, const char*,
                          unsigned int, unsigned int*) =
      isAccountConfig ? getDecryptionPasswordForAccountConfig
                      : getDecryptionPasswordFor;
  char*         password    = NULL;
  char*         fileContent = NULL;
  unsigned int  i           = 0;
  unsigned int* i_ptr       = &i;
  while (NULL == fileContent) {
    secFree(password);
    password = getPasswordFnc(file, pw_cmd, pw_file, pw_env,
                              0 /*default MAX_PASS_TRY*/, i_ptr);
    if (password == NULL && oidc_errno == OIDC_EMAXTRIES) {
      oidc_perror();
      return RESULT_WITH_PASSWORD_NULL;
    }
    fileContent = decryptFileContent(encrypted_content, password);
  }
  return (struct resultWithEncryptionPassword){.result   = fileContent,
                                               .password = password};
}

char* getDecryptedTextWithPromptFor(const char*   file,
                                    unsigned char isAccountConfig,
                                    const char* pw_cmd, const char* pw_file,
                                    const char* pw_env) {
  if (file == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct resultWithEncryptionPassword res =
      _getDecryptedTextAndPasswordWithPromptFor(file, isAccountConfig, pw_cmd,
                                                pw_file, pw_env);
  secFree(res.password);
  return res.result;
}
