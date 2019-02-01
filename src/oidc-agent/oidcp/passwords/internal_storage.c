#include "internal_storage.h"
#include "list/list.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"

#include <syslog.h>
#include <time.h>

struct password_entry {
  char*  shortname;
  char*  password;
  time_t expires_at;
};

void _secFreePasswordEntry(struct password_entry* pw) {
  secFree(pw->shortname);
  secFree(pw->password);
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

oidc_error_t internal_savePasswordFor(const char* shortname,
                                      const char* password, time_t expires_at) {
  if (shortname == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Saving password for '%s' in internal",
         shortname);
  if (passwords == NULL) {
    passwords        = list_new();
    passwords->free  = (void (*)(void*))_secFreePasswordEntry;
    passwords->match = (int (*)(void*, void*))matchPasswordEntryByShortname;
  }
  struct password_entry* pw = secAlloc(sizeof(struct password_entry));
  pw->shortname             = oidc_strcopy(shortname);
  pw->password              = oidc_strcopy(password);
  pw->expires_at            = expires_at;
  list_removeIfFound(
      passwords,
      pw);  // Removing an existing (old) entry for the same shortname -> update
  list_rpush(passwords, list_node_new(pw));
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Password for '%s' saved in internal",
         shortname);
  return OIDC_SUCCESS;
}

char* internal_getPasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Looking up password for '%s' in internal",
         shortname);
  if (passwords == NULL) {
    oidc_errno = OIDC_EPWNOTFOUND;
    return NULL;
  }
  struct password_entry key  = {.shortname = oidc_strcopy(shortname)};
  list_node_t*          node = findInList(passwords, &key);
  secFree(key.shortname);
  if (node == NULL) {
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "No password found for '%s' in internal",
           shortname);
    oidc_errno = OIDC_EPWNOTFOUND;
    return NULL;
  }
  struct password_entry* pwe = node->val;
  if (pwe->expires_at && pwe->expires_at < time(NULL)) {
    // Actually expired entries should already be gone from the list
    list_remove(passwords, node);
    oidc_errno = OIDC_EPWNOTFOUND;
    return NULL;
  }
  return oidc_strcopy(pwe->password);
}

oidc_error_t internal_removePasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "Removing password for '%s' from internal",
         shortname);
  if (passwords == NULL) {
    oidc_errno = OIDC_EPWNOTFOUND;
    return oidc_errno;
  }
  struct password_entry key = {.shortname = oidc_strcopy(shortname)};
  list_removeIfFound(passwords, &key);
  secFree(key.shortname);
  return OIDC_SUCCESS;
}
