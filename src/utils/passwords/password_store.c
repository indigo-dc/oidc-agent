#include "password_store.h"
#include "keyring.h"
#include "list/list.h"
#include "password_entry.h"
#include "password_handler.h"
#include "utils/deathUtils.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <syslog.h>
#include <time.h>

// TODO add encryption

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

static list_t* passwords = NULL;

char* memory_getPasswordFor(const struct password_entry* pwe) {
  if (pwe == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  if (pwe->expires_at && pwe->expires_at < time(NULL)) {
    // Actually expired entries should already be gone from the list
    oidc_errno = OIDC_EPWNOTFOUND;
    syslog(LOG_AUTHPRIV | LOG_NOTICE, "Found an expired entry for '%s'",
           pwe->shortname);
    return NULL;
  }
  return oidc_strcopy(pwe->password);
}

void initPasswordStore() {
  if (passwords == NULL) {
    passwords        = list_new();
    passwords->free  = (void (*)(void*))_secFreePasswordEntry;
    passwords->match = (int (*)(void*, void*))matchPasswordEntryByShortname;
  }
}

oidc_error_t savePassword(struct password_entry* pw) {
  if (pw == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Saving password for '%s'", pw->shortname);
  initPasswordStore();
  if (pw->type & PW_TYPE_MNG) {
    keyring_savePasswordFor(pw->shortname, pw->password);
  }
  // TODO encrypt the password field
  list_removeIfFound(
      passwords,
      pw);  // Removing an existing (old) entry for the same shortname -> update
  list_rpush(passwords, list_node_new(pw));
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Now there are %d passwords saved",
         passwords->len);
  return OIDC_SUCCESS;
}

oidc_error_t removePasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Removing password for '%s'", shortname);
  if (passwords == NULL) {
    oidc_errno = OIDC_EPWNOTFOUND;
    return oidc_errno;
  }
  struct password_entry key  = {.shortname = oidc_strcopy(shortname)};
  list_node_t*          node = findInList(passwords, &key);
  secFree(key.shortname);
  if (node == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No password found for '%s'", shortname);
    return OIDC_SUCCESS;
  }
  struct password_entry* pw   = node->val;
  unsigned char          type = pw->type;
  if (type & PW_TYPE_MNG) {
    keyring_removePasswordFor(shortname);
  }
  list_remove(passwords, node);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Now there are %d passwords saved",
         passwords->len);
  return OIDC_SUCCESS;
}

char* getPasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Getting password for '%s'", shortname);
  struct password_entry key  = {.shortname = oidc_strcopy(shortname)};
  list_node_t*          node = findInList(passwords, &key);
  secFree(key.shortname);
  if (node == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No password found for '%s'", shortname);
    return OIDC_SUCCESS;
  }
  struct password_entry* pw   = node->val;
  unsigned char          type = pw->type;
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Password type is %hhu", type);
  char* res = NULL;
  if (!res && type & PW_TYPE_MEM) {
    res = memory_getPasswordFor(pw);
  }
  if (!res && type & PW_TYPE_MNG) {
    res = keyring_getPasswordFor(shortname);
  }
  if (!res && type & PW_TYPE_CMD) {
    oidc_errno = OIDC_NOTIMPL;
    res        = NULL;
  }
  if (!res && type & PW_TYPE_PRMT) {
    oidc_errno = OIDC_NOTIMPL;
    res        = NULL;
  }
  return res;
}

time_t getMinPasswordDeath() {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Getting min death time for passwords");
  return getMinDeathFrom(passwords, (time_t(*)(void*))pwe_getExpiresAt);
}

struct password_entry* getDeathPasswordEntry() {
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Searching for death passwords");
  return getDeathElementFrom(passwords, (time_t(*)(void*))pwe_getExpiresAt);
}

void removeDeathPasswords() {
  struct password_entry* death_pwe = NULL;
  while ((death_pwe = getDeathPasswordEntry()) != NULL) {
    removePasswordFor(death_pwe->shortname);
  }
}
