#ifndef OIDC_DB_CODEVERIFIERS_H
#define OIDC_DB_CODEVERIFIERS_H

#include "db.h"

#define codeVerifierDB_new() \
  do { db_newDB(OIDC_DB_CODEVERIFIERS); } while (0)

#define codeVerifierDB_getList() db_getDB(OIDC_DB_CODEVERIFIERS)

#define codeVerifierDB_setMatchFunction(match) \
  db_setMatchFunction(OIDC_DB_CODEVERIFIERS, (match))

#define codeVerifierDB_setFreeFunction(free) \
  db_setFreeFunction(OIDC_DB_CODEVERIFIERS, (free))

#define codeVerifierDB_removeIfFound(value) \
  do { db_removeIfFound(OIDC_DB_CODEVERIFIERS, (value)); } while (0)

#define codeVerifierDB_addValue(value) \
  do { db_addValue(OIDC_DB_CODEVERIFIERS, (value)); } while (0)

#define codeVerifierDB_findValue(key) db_findValue(OIDC_DB_CODEVERIFIERS, (key))

#define codeVerifierDB_findValueWithFunction(key, function) \
  db_findValueWithFunction(OIDC_DB_CODEVERIFIERS, (key), (function))

#define codeVerifierDB_getMinDeath(getter) \
  db_getMinDeath(OIDC_DB_CODEVERIFIERS, (getter))

#define codeVerifierDB_getDeathEntry(getter) \
  db_getDeathEntry(OIDC_DB_CODEVERIFIERS, (getter))

#define codeVerifierDB_getSize() db_getSize(OIDC_DB_CODEVERIFIERS)

#define codeVerifierDB_reset() \
  do { db_reset(OIDC_DB_CODEVERIFIERS); } while (0)

#endif  // OIDC_DB_CODEVERIFIERS_H
