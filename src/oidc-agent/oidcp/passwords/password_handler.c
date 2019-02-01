#include "password_handler.h"
#include "keyring.h"
#include "list/list.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <syslog.h>
#include <time.h>

// TODO add encryption

struct password_entry {
  char*         shortname;
  unsigned char type;
  char*         password;
  time_t        expires_at;
  char*         command;
};

void _secFreePasswordEntry(struct password_entry* pw) {
  secFree(pw->shortname);
  secFree(pw->password);
  secFree(pw->command);
  secFree(pw);
}

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

oidc_error_t memory_savePasswordFor(struct password_entry* entry,
                                    const char* password, time_t expires_at) {
  if (entry == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Saving password for '%s' in memory",
         entry->shortname);
  secFree(entry->password);
  entry->password   = oidc_strcopy(password);
  entry->expires_at = expires_at;
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Password for '%s' saved in memory",
         entry->shortname);
  return OIDC_SUCCESS;
}

char* memory_getPasswordFor(const struct password_entry* pwe) {
  if (pwe == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  if (pwe->expires_at && pwe->expires_at < time(NULL)) {
    // Actually expired entries should already be gone from the list
    oidc_errno = OIDC_EPWNOTFOUND;
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

oidc_error_t savePasswordFor(const char* shortname, const char* password,
                             time_t expires_at, unsigned char type) {
  if (shortname == NULL || password == NULL || type == 0) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  initPasswordStore();
  struct password_entry* pw = secAlloc(sizeof(struct password_entry));
  pw->shortname             = oidc_strcopy(shortname);
  pw->type                  = type;
  if (type & PW_TYPE_MEM) {
    memory_savePasswordFor(pw, password, expires_at);
  }
  if (type & PW_TYPE_CMD) {
    pw->command = oidc_strcopy(
        password);  // When using a command, no password will be provided
  }
  if (type & PW_TYPE_MNG) {
    keyring_savePasswordFor(shortname, password);
  }
  list_removeIfFound(
      passwords,
      pw);  // Removing an existing (old) entry for the same shortname -> update
  list_rpush(passwords, list_node_new(pw));
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
  return OIDC_SUCCESS;
}

char* getPasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
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
  char*                  res  = NULL;
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
