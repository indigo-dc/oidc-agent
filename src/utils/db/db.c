#include "db.h"
#include "list/list.h"
#include "utils/deathUtils.h"
#include "utils/memory.h"

#include <syslog.h>

static list_t* dbs = NULL;

struct oidc_db {
  db_name db;
  list_t* list;
};

int matchDBs(const struct oidc_db* a, const struct oidc_db* b) {
  return a->db == b->db;
}

void db_init() {
  if (dbs != NULL) {
    return;
  }
  dbs        = list_new();
  dbs->match = (matchFunction)matchDBs;
}

list_node_t* _getDBNode(const db_name db) {
  if (dbs == NULL) {
    return NULL;
  }
  struct oidc_db key = {.db = db};
  return findInList(dbs, &key);
}

list_t* db_getDB(const db_name db) {
  list_node_t* found = _getDBNode(db);
  if (found == NULL) {
    return NULL;
  }
  return ((struct oidc_db*)found->val)->list;
}

void db_newDB(const db_name db) {
  db_init();
  if (db_getDB(db) != NULL) {
    return;
  }
  struct oidc_db* db_e = secAlloc(sizeof(struct oidc_db));
  db_e->db             = db;
  db_e->list           = list_new();
  list_rpush(dbs, list_node_new(db_e));
}

matchFunction db_setMatchFunction(const db_name db, matchFunction match) {
  db_init();
  list_t* db_list = db_getDB(db);
  if (db_list == NULL) {
    db_newDB(db);
    return db_setMatchFunction(db, match);
  }
  matchFunction oldMatch = db_list->match;
  db_list->match         = match;
  return oldMatch;
}

freeFunction db_setFreeFunction(const db_name db, void (*free_fn)(void*)) {
  db_init();
  list_t* db_list = db_getDB(db);
  if (db_list == NULL) {
    db_newDB(db);
    return db_setFreeFunction(db, free_fn);
  }
  freeFunction oldFree = db_list->free;
  db_list->free        = free_fn;
  return oldFree;
}

void db_removeIfFound(const db_name db, void* value) {
  list_removeIfFound(db_getDB(db), value);
}

void db_addValue(const db_name db, void* value) {
  list_rpush(db_getDB(db), list_node_new(value));
  syslog(LOG_AUTHPRIV | LOG_DEBUG,
         "Added value to db %hhu. Now there are %lu entries.", db,
         db_getSize(db));
}

size_t db_getSize(const db_name db) {
  list_t* dbl = db_getDB(db);
  return dbl ? dbl->len : 0;
}

void* db_findValue(const db_name db, void* key) {
  list_node_t* node = findInList(db_getDB(db), key);
  return node ? node->val : NULL;
}

list_t* db_findAllValues(const db_name db, void* key) {
  return findAllInList(db_getDB(db), key);
}

void* db_findValueWithFunction(const db_name db, void* key,
                               matchFunction match) {
  matchFunction oldMatch = db_setMatchFunction(db, match);
  void*         ret      = db_findValue(db, key);
  db_setMatchFunction(db, oldMatch);
  return ret;
}

void db_reset(const db_name db) {
  list_node_t* node = _getDBNode(db);
  if (node == NULL) {
    return;
  }
  struct oidc_db* db_s   = node->val;
  list_t*         list   = db_s->list;
  matchFunction   match  = list->match;
  void (*free_fn)(void*) = list->free;
  secFreeList(list);
  db_s->list        = list_new();
  db_s->list->match = match;
  db_s->list->free  = free_fn;
}

time_t db_getMinDeath(const db_name db, time_t (*deathGetter)(void*)) {
  return getMinDeathFrom(db_getDB(db), deathGetter);
}

void* db_getDeathEntry(const db_name db, time_t (*deathGetter)(void*)) {
  return getDeathElementFrom(db_getDB(db), deathGetter);
}
