#include "accountUtils.h"
#include "account/account.h"
#include "deathUtils.h"
#include "defines/settings.h"
#include "utils/file_io/file_io.h"
#include "utils/prompt.h"

#include <syslog.h>
#include <time.h>

/**
 * @brief returns the minimum death time in an account list
 * @param accounts a list of (loaded) accounts
 * @return the minimum time of death; might be @c 0
 */
time_t getMinAccountDeath(list_t* accounts) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Getting min death time for accounts");
  return getMinDeathFrom(accounts, (time_t(*)(void*))account_getDeath);
}

/**
 * @brief returns an account that death was prior to the current time
 * @param accounts a list of (loaded) accounts - searchspace
 * only one death account is returned per call; to find all death accounts in @p
 * accounts @c getDeathAccount should be called until it returns @c NULL
 * @return a pointer to a dead account or @c NULL
 */
struct oidc_account* getDeathAccount(list_t* accounts) {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Searching for death accounts");
  return getDeathElementFrom(accounts, (time_t(*)(void*))account_getDeath);
}

/**
 * @brief creates account from config file.
 * The config file is provided by the user. It might be a clientconfig file
 * created and encrypted by oidc-gen or an unencrypted file.
 * @param filename the absolute path of the account config file
 * @return a pointer to the result oidc_account struct. Has to be freed after
 * usage using \f secFree
 */
struct oidc_account* accountFromFile(const char* filename) {
  char* inputconfig = readFile(filename);
  if (!inputconfig) {
    printError("Could not read config file: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Read config from user provided file: %s",
         inputconfig);
  struct oidc_account* account = getAccountFromJSON(inputconfig);
  if (!account) {
    char* encryptionPassword = NULL;
    int   i;
    for (i = 0; i < MAX_PASS_TRIES && account == NULL; i++) {
      syslog(LOG_AUTHPRIV | LOG_DEBUG,
             "Read config from user provided file: %s", inputconfig);
      encryptionPassword =
          promptPassword("Enter decryption Password for client config file: ");
      account = decryptAccountText(inputconfig, encryptionPassword);
      secFree(encryptionPassword);
      if (account == NULL) {
        oidc_perror();
      }
    }
  }
  secFree(inputconfig);
  return account;
}
