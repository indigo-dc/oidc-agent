#ifndef OIDC_DB_FILES_H
#define OIDC_DB_FILES_H

#include "db.h"

void fileDB_new();

#define fileDB_setMatchFunction(match) \
  db_setMatchFunction(OIDC_DB_FILES, (match))

void fileDB_removeIfFound(const char* key);

void fileDB_addValue(const char* key, const char* data);

char* fileDB_findValue(const char* key);

#define fileDB_getSize() db_getSize(OIDC_DB_FILES)

#define fileDB_reset() \
  do { db_reset(OIDC_DB_FILES); } while (0)

#endif  // OIDC_DB_FILES_H
