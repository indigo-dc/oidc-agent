#ifndef OIDC_DB_ACCOUNTS_H
#define OIDC_DB_ACCOUNTS_H

#include "db.h"

#define accountDB_new() \
  do { db_newDB(OIDC_DB_ACCOUNTS); } while (0)

#define accountDB_getList() db_getDB(OIDC_DB_ACCOUNTS)

#define accountDB_setMatchFunction(match) \
  db_setMatchFunction(OIDC_DB_ACCOUNTS, (match))

#define accountDB_setFreeFunction(free) \
  db_setFreeFunction(OIDC_DB_ACCOUNTS, (free))

#define accountDB_removeIfFound(value) \
  do { db_removeIfFound(OIDC_DB_ACCOUNTS, (value)); } while (0)

#define accountDB_addValue(value) \
  do { db_addValue(OIDC_DB_ACCOUNTS, (value)); } while (0)

#define accountDB_findValue(key) db_findValue(OIDC_DB_ACCOUNTS, (key))

#define accountDB_findAllValues(key) db_findAllValues(OIDC_DB_ACCOUNTS, (key))

#define accountDB_findValueWithFunction(key, function) \
  db_findValueWithFunction(OIDC_DB_ACCOUNTS, (key), (function))

#define accountDB_getMinDeath(getter) db_getMinDeath(OIDC_DB_ACCOUNTS, (getter))

#define accountDB_getDeathEntry(getter) \
  db_getDeathEntry(OIDC_DB_ACCOUNTS, (getter))

#define accountDB_getSize() db_getSize(OIDC_DB_ACCOUNTS)

#define accountDB_reset() \
  do { db_reset(OIDC_DB_ACCOUNTS); } while (0)

#endif  // OIDC_DB_ACCOUNTS_H
