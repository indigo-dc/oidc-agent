#include "oidc_file_io.h"

#include "file_io.h"

#include "../../lib/list/src/list.h"

#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <syslog.h>

char* possibleLocations[] = {"~/.config/oidc-agent/", "~/.oidc-agent/"};

/** @fn char* readOidcFile(const char* filename)
 * @brief reads a file located in the oidc dir and returns a pointer to the
 * content
 * @param filename the filename of the file
 * @return a pointer to the file content. Has to be freed after usage.
 */
char* readOidcFile(const char* filename) {
  char* path = concatToOidcDir(filename);
  char* c    = readFile(path);
  secFree(path);
  return c;
}

/** @fn void writeOidcFile(const char* filename, const char* text)
 * @brief writes text to a file located in the oidc directory
 * @note \p text has to be nullterminated and must not contain nullbytes.
 * @param filename the file to be written
 * @param text the nullterminated text to be written
 * @return OIDC_OK on success, OID_EFILE if an error occured. The system sets
 * errno.
 */
oidc_error_t writeOidcFile(const char* filename, const char* text) {
  char*        path = concatToOidcDir(filename);
  oidc_error_t er   = writeFile(path, text);
  secFree(path);
  return er;
}

/** @fn int oidcFileDoesExist(const char* filename)
 * @brief checks if a file exists in the oidc dir
 * @param filename the file to be checked
 * @return 1 if the file does exist, 0 if not
 */
int oidcFileDoesExist(const char* filename) {
  char* path = concatToOidcDir(filename);
  int   b    = fileDoesExist(path);
  secFree(path);
  return b;
}

/** @fn char* getOidcDir()
 * @brief get the oidc directory path
 * @return a pointer to the oidc directory path. Has to be freed after usage. If
 * no oidc dir is found, NULL is returned
 */
char* getOidcDir() {
  char*        home = getenv("HOME");
  unsigned int i;
  for (i = 0; i < sizeof(possibleLocations) / sizeof(*possibleLocations); i++) {
    char* path = oidc_strcat(home, possibleLocations[i] + 1);
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Checking if dir '%s' exists.", path);
    if (dirExists(path) > 0) {
      return path;
    }
    secFree(path);
  }
  return NULL;
}

/** @fn int removeOidcFile(const char* filename)
 * @brief removes a file located in the oidc dir
 * @param filename the filename of the file to be removed
 * @return On success, 0 is returned.  On error, -1 is returned, and errno is
 * set appropriately.
 */
int removeOidcFile(const char* filename) {
  char* path = concatToOidcDir(filename);
  int   r    = removeFile(path);
  secFree(path);
  return r;
}

char* concatToOidcDir(const char* filename) {
  char* oidc_dir = getOidcDir();
  char* path     = oidc_strcat(oidc_dir, filename);
  secFree(oidc_dir);
  return path;
}

list_t* getFileListForDirIf(const char* dirname,
                            int(match(const char*, const char*)),
                            const char* arg) {
  DIR*           dir;
  struct dirent* ent;
  if ((dir = opendir(dirname)) != NULL) {
    list_t* list = list_new();
    list->free   = (void (*)(void*)) & _secFree;
    list->match  = (int (*)(void*, void*)) & strequal;
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
#ifdef _DIRENT_HAVE_DTYPE
        if (ent->d_type == DT_REG) {
          if (match(ent->d_name, arg)) {
            list_rpush(list, list_node_new(oidc_strcopy(ent->d_name)));
          }
        }
#else
        if (match(ent->d_name, arg)) {
          list_rpush(list, list_node_new(oidc_strcopy(ent->d_name)));
        }
#endif
      }
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
  char*   oidc_dir = getOidcDir();
  list_t* list     = getFileListForDirIf(oidc_dir, &isAccountConfigFile, NULL);
  secFree(oidc_dir);
  return list;
}

list_t* getClientConfigFileList() {
  char*        oidc_dir = getOidcDir();
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
