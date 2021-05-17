#include "fileUtils.h"
#include "oidc_file_io.h"
#include "utils/crypt/crypt.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/printer.h"
#include "utils/stringUtils.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief checks if the oidc directory exists
 */
void checkOidcDirExists() {
  char* dir = NULL;
  if ((dir = getOidcDir()) == NULL) {
    printError("Error: oidc-dir does not exist. Run oidc-gen to create it.\n");
    exit(EXIT_FAILURE);
  }
  secFree(dir);
}

/**
 * @brief asserts that the oidc directory exists
 */
void assertOidcDirExists() {
  char* dir = getOidcDir();
  if (dir != NULL) {
    secFree(dir);
    return;
  }
  logger(DEBUG, "oidcdir does not exist, creating oidcdir");
  if (createOidcDir() != OIDC_SUCCESS) {
    oidc_perror();
    exit(EXIT_FAILURE);
  }
}

list_t* getFileListForDirIf(const char* dirname,
                            int(match(const char*, const char*)),
                            const char* arg) {
  DIR*           dir;
  struct dirent* ent;
  if ((dir = opendir(dirname)) != NULL) {
    list_t* list = list_new();
    list->free   = (void (*)(void*)) & _secFree;
    list->match  = (matchFunction)strequal;
    while ((ent = readdir(dir)) != NULL) {
#ifdef _DIRENT_HAVE_DTYPE
      if (ent->d_type == DT_REG) {
#endif
        if (!strstarts(ent->d_name, ".")) {
          if (match(ent->d_name, arg)) {
            list_rpush(list, list_node_new(oidc_strcopy(ent->d_name)));
          }
        }
#ifdef _DIRENT_HAVE_DTYPE
      }
#endif
    }
    closedir(dir);
    return list;
  } else {
    oidc_seterror(strerror(errno));
    oidc_errno = OIDC_EERROR;
    return NULL;
  }
}

int alwaysOne(const char* a __attribute__((unused)),
              const char* b __attribute__((unused))) {
  return 1;
}

list_t* getFileListForDir(const char* dirname) {
  return getFileListForDirIf(dirname, &alwaysOne, NULL);
}

int isClientConfigFile(const char* filename,
                       const char* a __attribute__((unused))) {
  const char* const suffix = ".clientconfig";
  if (strEnds(filename, suffix)) {
    return 1;
  }
  char* pos = NULL;
  if ((pos = strstr(filename, suffix))) {
    pos += strlen(suffix);
    while (*pos != '\0') {
      if (!isdigit(*pos)) {
        return 0;
      }
      pos++;
    }
    return 1;
  }
  return 0;
}

int isAccountConfigFile(const char* filename,
                        const char* a __attribute__((unused))) {
  if (isClientConfigFile(filename, a)) {
    return 0;
  }
  if (strEnds(filename, ".config")) {
    return 0;
  }
  return 1;
}

list_t* getAccountConfigFileList() {
  char* oidc_dir = getOidcDir();
  if (oidc_dir == NULL) {
    return NULL;
  }
  list_t* list = getFileListForDirIf(oidc_dir, &isAccountConfigFile, NULL);
  secFree(oidc_dir);
  return list;
}

list_t* getClientConfigFileList() {
  char* oidc_dir = getOidcDir();
  if (oidc_dir == NULL) {
    return NULL;
  }
  list_t*      list = getFileListForDirIf(oidc_dir, &isClientConfigFile, NULL);
  list_node_t* node;
  list_iterator_t* it = list_iterator_new(list, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* old = node->val;
    node->val = oidc_strcat(oidc_dir, old);
    secFree(old);
  }
  secFree(oidc_dir);
  return list;
}

int compareFilesByName(const char* filename1, const char* filename2) {
  return strcmp(filename1, filename2);
}

int compareOidcFilesByDateModified(const char* filename1,
                                   const char* filename2) {
  struct stat* stat1 = secAlloc(sizeof(struct stat));
  struct stat* stat2 = secAlloc(sizeof(struct stat));
  char*        path1 = concatToOidcDir(filename1);
  char*        path2 = concatToOidcDir(filename2);
  stat(path1, stat1);
  stat(path2, stat2);
  int ret = 0;
  if (stat1->st_mtime < stat2->st_mtime) {
    ret = -1;
  }
  if (stat1->st_mtime > stat2->st_mtime) {
    ret = 1;
  }
  secFree(stat1);
  secFree(stat2);
  return ret;
}

int compareOidcFilesByDateAccessed(const char* filename1,
                                   const char* filename2) {
  struct stat* stat1 = secAlloc(sizeof(struct stat));
  struct stat* stat2 = secAlloc(sizeof(struct stat));
  char*        path1 = concatToOidcDir(filename1);
  char*        path2 = concatToOidcDir(filename2);
  stat(path1, stat1);
  stat(path2, stat2);
  int ret = 0;
  if (stat1->st_atime < stat2->st_atime) {
    ret = -1;
  }
  if (stat1->st_atime > stat2->st_atime) {
    ret = 1;
  }
  secFree(stat1);
  secFree(stat2);
  return ret;
}

char* generateClientConfigFileName(const char* issuer_url,
                                   const char* client_id) {
  char* filename_fmt = "%s_%s_%s.clientconfig";
  char* iss          = oidc_strcopy(issuer_url + 8);
  char* iss_new_end  = strchr(iss, '/');  // cut after the first '/'
  *iss_new_end       = 0;
  char* today        = getDateString();
  char* filename     = oidc_sprintf(filename_fmt, iss, today, client_id);
  secFree(today);
  secFree(iss);

  if (oidcFileDoesExist(filename)) {
    logger(DEBUG, "The clientconfig file already exists. Changing path.");
    int   i       = 0;
    char* newName = NULL;
    do {
      secFree(newName);
      newName = oidc_sprintf("%s%d", filename, i);
      i++;
    } while (oidcFileDoesExist(newName));
    secFree(filename);
    filename = newName;
  }
  return filename;
}

oidc_error_t changeGroup(const char* path, const char* group_name) {
  if (path == NULL || group_name == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  errno             = 0;
  struct group* grp = getgrnam(group_name);
  if (grp == NULL) {
    if (errno == 0) {
      oidc_errno = OIDC_EGROUPNF;
    } else {
      oidc_setErrnoError();
    }
    return oidc_errno;
  }
  gid_t gid = grp->gr_gid;
  if (chown(path, -1, gid) != 0) {
    oidc_setErrnoError();
    return oidc_errno;
  }
  struct stat* buf = secAlloc(sizeof(struct stat));
  if (stat(path, buf) != 0) {
    oidc_setErrnoError();
    secFree(buf);
    return oidc_errno;
  }
  int err = chmod(path, buf->st_mode | S_ISGID | S_IRWXG);
  secFree(buf);
  if (err != 0) {
    oidc_setErrnoError();
    return oidc_errno;
  }
  return OIDC_SUCCESS;
}
