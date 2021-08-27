#ifndef OIDC_DB_DEVICECODES_H
#define OIDC_DB_DEVICECODES_H

#include "db.h"

#define deviceCodeDB_new() \
  do { db_newDB(OIDC_DB_DEVICECODES); } while (0)

#define deviceCodeDB_getList() db_getDB(OIDC_DB_DEVICECODES)

#define deviceCodeDB_setMatchFunction(match) \
  db_setMatchFunction(OIDC_DB_DEVICECODES, (match))

#define deviceCodeDB_setFreeFunction(free) \
  db_setFreeFunction(OIDC_DB_DEVICECODES, (free))

#define deviceCodeDB_removeIfFound(value) \
  do { db_removeIfFound(OIDC_DB_DEVICECODES, (value)); } while (0)

#define deviceCodeDB_addValue(value) \
  do { db_addValue(OIDC_DB_DEVICECODES, (value)); } while (0)

#define deviceCodeDB_findValue(key) db_findValue(OIDC_DB_DEVICECODES, (key))

#define deviceCodeDB_findAllValues(key) \
  db_findAllValues(OIDC_DB_DEVICECODES, (key))

#define deviceCodeDB_findValueWithFunction(key, function) \
  db_findValueWithFunction(OIDC_DB_DEVICECODES, (key), (function))

#define deviceCodeDB_getMinDeath(getter) \
  db_getMinDeath(OIDC_DB_DEVICECODES, (getter))

#define deviceCodeDB_getDeathEntry(getter) \
  db_getDeathEntry(OIDC_DB_DEVICECODES, (getter))

#define deviceCodeDB_getSize() db_getSize(OIDC_DB_DEVICECODES)

#define deviceCodeDB_reset() \
  do { db_reset(OIDC_DB_DEVICECODES); } while (0)

#endif  // OIDC_DB_DEVICECODES_H
