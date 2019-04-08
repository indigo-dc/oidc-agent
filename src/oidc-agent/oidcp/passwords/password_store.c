#include "password_store.h"
#include "oidc-agent/oidcp/passwords/askpass.h"
#include "oidc-agent/oidcp/passwords/keyring.h"
#include "oidc-agent/oidcp/passwords/password_handler.h"
#include "utils/crypt/passwordCrypt.h"
#include "utils/db/password_db.h"
#include "utils/deathUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/password_entry.h"
#include "utils/system_runner.h"

#include "utils/logger.h"
#include <time.h>

int matchPasswordEntryByShortname(struct password_entry* a,
                                  struct password_entry* b) {
  if (a == NULL && b == NULL) {
    return 1;
  }
  if (a == NULL || b == NULL) {
    return 0;
  }
  if (a->shortname == NULL && b->shortname == NULL) {
    return 1;
  }
  if (a->shortname == NULL || b->shortname == NULL) {
    return 0;
  }
  return strequal(a->shortname, b->shortname);
}

char* memory_getPasswordFor(const struct password_entry* pwe) {
  if (pwe == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (pwe->expires_at && pwe->expires_at < time(NULL)) {
    // Actually expired entries should already be gone from the list
    oidc_errno = OIDC_EPWNOTFOUND;
    logger(NOTICE, "Found an expired entry for '%s'",
           pwe->shortname);
    return NULL;
  }
  return oidc_strcopy(pwe->password);
}

void initPasswordStore() {
  passwordDB_new();
  passwordDB_setMatchFunction((matchFunction)matchPasswordEntryByShortname);
  passwordDB_setFreeFunction((void (*)(void*))_secFreePasswordEntry);
}

oidc_error_t savePassword(struct password_entry* pw) {
  if (pw == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  logger(DEBUG, "Saving password for '%s'", pw->shortname);
  initPasswordStore();
  if (pw->password) {  // For prompt and command password won't be set
    char* tmp = encryptPassword(pw->password, pw->shortname);
    if (tmp == NULL) {
      return oidc_errno;
    }
    pwe_setPassword(pw, tmp);
  }
  if (pw->command) {
    char* tmp = encryptPassword(pw->command, pw->shortname);
    if (tmp == NULL) {
      return oidc_errno;
    }
    pwe_setCommand(pw, tmp);
  }
  if (pw->type & PW_TYPE_MNG) {
    keyring_savePasswordFor(pw->shortname, pw->password);
  }
  passwordDB_removeIfFound(
      pw);  // Removing an existing (old) entry for the same shortname -> update
  passwordDB_addValue(pw);
  logger(DEBUG, "Now there are %lu passwords saved",
         passwordDB_getSize());
  return OIDC_SUCCESS;
}

oidc_error_t removeOrExpirePasswordFor(const char* shortname, int remove) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  logger(DEBUG, "%s password for '%s'",
         remove ? "Removing" : "Expiring", shortname);
  struct password_entry  key = {.shortname = oidc_strcopy(shortname)};
  struct password_entry* pw  = passwordDB_findValue(&key);
  secFree(key.shortname);
  if (pw == NULL) {
    logger(DEBUG, "No password found for '%s'", shortname);
    return OIDC_SUCCESS;
  }
  unsigned char type = pw->type;
  if (type & PW_TYPE_MNG) {
    keyring_removePasswordFor(shortname);
  }
  if (remove) {
    passwordDB_removeIfFound(pw);
  } else {
    pwe_setPassword(pw, NULL);
    pwe_setExpiresAt(pw, 0);
  }
  logger(DEBUG, "Now there are %lu passwords saved",
         passwordDB_getSize());
  return OIDC_SUCCESS;
}

oidc_error_t removePasswordFor(const char* shortname) {
  return removeOrExpirePasswordFor(shortname, 1);
}

oidc_error_t expirePasswordFor(const char* shortname) {
  return removeOrExpirePasswordFor(shortname, 0);
}

oidc_error_t removeAllPasswords() {
  logger(DEBUG, "Removing all passwords");
  passwordDB_reset();
  return OIDC_SUCCESS;
}

char* getPasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  logger(DEBUG, "Getting password for '%s'", shortname);
  struct password_entry  key = {.shortname = oidc_strcopy(shortname)};
  struct password_entry* pw  = passwordDB_findValue(&key);
  secFree(key.shortname);
  if (pw == NULL) {
    logger(DEBUG, "No password found for '%s'", shortname);
    logger(DEBUG, "Try getting password from user prompt");
    return askpass_getPasswordForUpdate(shortname);
  }
  unsigned char type = pw->type;
  logger(DEBUG, "Password type is %hhu", type);
  char* res = NULL;
  if (!res && type & PW_TYPE_MEM) {
    logger(DEBUG, "Try getting password from memory");
    char* crypt = memory_getPasswordFor(pw);
    res         = decryptPassword(crypt, shortname);
    secFree(crypt);
  }
  if (!res && type & PW_TYPE_MNG) {
    logger(DEBUG, "Try getting password from keyring");
    char* crypt = keyring_getPasswordFor(shortname);
    res         = decryptPassword(crypt, shortname);
    secFree(crypt);
  }
  if (!res && type & PW_TYPE_CMD) {
    logger(DEBUG, "Try getting password from command");
    char* cmd = decryptPassword(pw->command, shortname);
    res       = getOutputFromCommand(cmd);
    secFree(cmd);
  }
  if (!res && type & PW_TYPE_PRMT) {
    logger(DEBUG, "Try getting password from user prompt");
    res = askpass_getPasswordForUpdate(shortname);
    if (res && type & PW_TYPE_MEM) {
      pwe_setPassword(pw, encryptPassword(res, shortname));
    }
  }
  return res;
}

time_t getMinPasswordDeath() {
  logger(DEBUG, "Getting min death time for passwords");
  return passwordDB_getMinDeath((time_t(*)(void*))pwe_getExpiresAt);
}

struct password_entry* getDeathPasswordEntry() {
  logger(DEBUG, "Searching for death passwords");
  return passwordDB_getDeathEntry((time_t(*)(void*))pwe_getExpiresAt);
}

void removeDeathPasswords() {
  struct password_entry* death_pwe = NULL;
  while ((death_pwe = getDeathPasswordEntry()) != NULL) {
    expirePasswordFor(death_pwe->shortname);
  }
}
