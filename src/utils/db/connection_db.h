#ifndef OIDC_DB_CONNECTIONS_H
#define OIDC_DB_CONNECTIONS_H

#include "db.h"

#define connectionDB_new() \
  do { db_newDB(OIDC_DB_CONNECTIONS); } while (0)

#define connectionDB_getList() db_getDB(OIDC_DB_CONNECTIONS)

#define connectionDB_setMatchFunction(match) \
  db_setMatchFunction(OIDC_DB_CONNECTIONS, (match))

#define connectionDB_setFreeFunction(free) \
  db_setFreeFunction(OIDC_DB_CONNECTIONS, (free))

#define connectionDB_removeIfFound(value) \
  do { db_removeIfFound(OIDC_DB_CONNECTIONS, (value)); } while (0)

#define connectionDB_addValue(value) \
  do { db_addValue(OIDC_DB_CONNECTIONS, (value)); } while (0)

#define connectionDB_findValue(key) db_findValue(OIDC_DB_CONNECTIONS, (key))

#define connectionDB_getMinDeath(getter) \
  db_getMinDeath(OIDC_DB_CONNECTIONS, (getter))

#define connectionDB_getDeathEntry(getter) \
  db_getDeathEntry(OIDC_DB_CONNECTIONS, (getter))

#define connectionDB_getSize() db_getSize(OIDC_DB_CONNECTIONS)

#define connectionDB_reset() \
  do { db_reset(OIDC_DB_CONNECTIONS); } while (0)

#endif  // OIDC_DB_CONNECTIONS_H
