#include "file_db.h"
#include "utils/crypt/memoryCrypt.h"
#include "utils/matcher.h"
#include "utils/memory.h"
#include "utils/oidc_string.h"
#include "utils/stringUtils.h"

struct file_dummy {
  char* filename;
  char* data;
};

void secFreeFileDummy(struct file_dummy* fd) {
  if (fd == NULL) {
    return;
  }
  secFree(fd->filename);
  secFree(fd->data);
  secFree(fd);
}

int _fd_match(const struct file_dummy* fd1, const struct file_dummy* fd2) {
  return matchStrings(fd1 ? fd1->filename : NULL, fd2 ? fd2->filename : NULL);
}

void fileDB_new() {
  db_newDB(OIDC_DB_FILES);
  db_setFreeFunction(OIDC_DB_FILES, (freeFunction)secFreeFileDummy);
  db_setMatchFunction(OIDC_DB_FILES, (matchFunction)_fd_match);
}

void fileDB_addValue(const char* key, const char* data) {
  struct file_dummy* value = secAlloc(sizeof(struct file_dummy));
  value->filename          = oidc_strcopy(key);
  value->data              = memoryEncrypt(data);
  db_addValue(OIDC_DB_FILES, value);
}

struct file_dummy* _findValue(const char* filename) {
  char*              tmp = oidc_strcopy(filename);
  struct file_dummy  key = {.filename = tmp};
  struct file_dummy* fd  = db_findValue(OIDC_DB_FILES, (&key));
  secFree(tmp);
  return fd;
}

char* fileDB_findValue(const char* filename) {
  struct file_dummy* fd = _findValue(filename);
  return fd ? memoryDecrypt(fd->data) : NULL;
}

void fileDB_removeIfFound(const char* filename) {
  db_removeIfFound(OIDC_DB_FILES, _findValue(filename));
}
