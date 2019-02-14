#ifndef OIDC_DB_PASSWORDS_H
#define OIDC_DB_PASSWORDS_H

#include "db.h"

#define passwordDB_new() \
  do { db_newDB(OIDC_DB_PASSWORDS); } while (0)

#define passwordDB_setMatchFunction(match) \
  db_setMatchFunction(OIDC_DB_PASSWORDS, (match))

#define passwordDB_setFreeFunction(free) \
  db_setFreeFunction(OIDC_DB_PASSWORDS, (free))

#define passwordDB_removeIfFound(value) \
  do { db_removeIfFound(OIDC_DB_PASSWORDS, (value)); } while (0)

#define passwordDB_addValue(value) \
  do { db_addValue(OIDC_DB_PASSWORDS, (value)); } while (0)

#define passwordDB_findValue(key) db_findValue(OIDC_DB_PASSWORDS, (key))

#define passwordDB_getMinDeath(getter) \
  db_getMinDeath(OIDC_DB_PASSWORDS, (getter))

#define passwordDB_getDeathEntry(getter) \
  db_getDeathEntry(OIDC_DB_PASSWORDS, (getter))

#define passwordDB_getSize() db_getSize(OIDC_DB_PASSWORDS)

#define passwordDB_reset() \
  do { db_reset(OIDC_DB_PASSWORDS); } while (0)

#endif  // OIDC_DB_PASSWORDS_H
