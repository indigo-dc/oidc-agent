#define _XOPEN_SOURCE 500

#include "oidc_utilities.h"
#include "oidc_error.h"
#include "settings.h"
#include "file_io.h"
#include "crypt.h"

#include "../lib/list/src/list.h"

#include <time.h>
#include <stdio.h>
#include <strings.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>

/** @fn int isValid(const char* c)
 * @brief checks if a string contains a valid value, meaning it is not empty,
 * NULL or "(null)"
 * @param c the string to be checked
 * @return 1 if the string is valid; 0 if not
 */
int isValid(const char* c) {
  return c && strcmp("", c)!=0 && strcmp("(null)", c)!=0;
}

/** @fn strstarts(const char* str, const char* pre)
 * @brief checks if a string starts with a given string
 * @param str the string to be checked
 * @param pre the prefix \p str might start with
 * @return 1 if str starts with pre; 0 if not
 */
int strstarts(const char* str, const char* pre) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

int strEnds(const char* str, const char* suf) {
  if (str==NULL || suf==NULL) {
    return 0;
  }
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suf);
  if (lensuffix >  lenstr) {
    return 0;
  }
  return strncmp(str + lenstr - lensuffix, suf, lensuffix) == 0;
}

int strEndsNot(const char* str, const char* suf) {
  return strEnds(str, suf) == 0 ? 1 : 0;
}

/** @fn void clearFree(void* p, size_t len)
 * @brief clears and frees allocated memory.
 * @param p a pointer to the memory to be freed
 * @param len the length of the allocated memory
 */
void clearFree(void* p, size_t len) {
  if(p==NULL) {
    return;
  }
  memset(p, 0, len);
  free(p);
}

/** @fn void clearFreeString(char* s)
 * @brief clears and frees an allocated string.
 * @param s a pointer to the string to be freed
 */
void clearFreeString(char* s) {
  if(s==NULL) {
    return;
  }
  clearFree(s, strlen(s));
}

/** @fn char* getDateString()
 * @brief returns the current date in YYYY-mm-dd format
 * @returns a pointer to the formated date. Has to be freed after usage
 */
char* getDateString() {
  char* s = calloc(sizeof(char), 10+1);
  time_t now = time(NULL);
  struct tm *t = localtime(&now);


  strftime(s, 10+1, "%F", t);
  return s;
}

char* oidc_sprintf(const char* fmt, ...) {
  va_list args, orig;
  va_start(args, fmt);
  va_start(orig, fmt);
  char* s = calloc(sizeof(char), vsnprintf(NULL, 0, fmt, args)+1);
  if (s==NULL) {
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  vsprintf(s, fmt, orig);
  return s;
}

char* oidc_strcat(const char* str, const char* suf) {
  return oidc_sprintf("%s%s", str, suf);
}
char* oidc_strcopy(const char* str) {
  return oidc_sprintf("%s", str);
}



long random_at_most(long max) {
  // max <= RAND_MAX < ULONG_MAX, so this is okay.
  unsigned long num_bins = (unsigned long) max + 1,
                num_rand = (unsigned long) RAND_MAX + 1,
                bin_size = num_rand / num_bins,
                defect   = num_rand % num_bins;

  long x;
  do {
    x = random();
  } while (num_rand - defect <= (unsigned long)x);

  return x/bin_size;
}

unsigned short getRandomPort() {
  unsigned short maxPort = 49151;
  unsigned short minPort = 1024;
  return random_at_most(maxPort-minPort) + minPort;
}

char* portToUri(unsigned short port) {
  return oidc_sprintf("http://localhost:%hu", port);
}

unsigned short getPortFromUri(const char* uri) {
  unsigned short s;
  sscanf(uri, "http://localhost:%hu", &s);
  return s;
}

char* arrToListString(char** arr, size_t size, char delimiter, int surround) {
  if(arr==NULL || size==0) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  char* str = oidc_sprintf("%s", arr[0]);
  if(str==NULL){
    return NULL;
  }
  char* tmp = NULL;
  unsigned int i;
  for(i=1; i<size; i++){
    tmp = oidc_sprintf("%s%c%s", str, delimiter, arr[i]);
    clearFreeString(str);
    if(tmp==NULL){
      return NULL;
    }
    str = tmp;
  }

  if(!surround) {
    return str;
  }
  tmp = oidc_sprintf("[%s]", str);
  clearFreeString(str);
  if(tmp==NULL){
    return NULL;
  }
  return tmp;
}

char* delimitedListToJSONArray(char* str, char delimiter) {
  if(str==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  size_t size = strCountChar(str, delimiter)+1;
  char* copy = oidc_sprintf("%s", str);
  int len = strlen(copy);
  char* delim = oidc_sprintf("%c", delimiter);
  char* json = oidc_sprintf("\"%s\"", strtok(copy, delim));
  size_t i;
  for(i=1; i<size; i++) {
    char* tmp = oidc_sprintf("%s, \"%s\"", json, strtok(NULL, delim));
    clearFreeString(json);
    if(tmp==NULL) {
      clearFreeString(delim);
      clearFree(copy, len);
      return NULL;
    }
    json = tmp;
  }
  clearFreeString(delim);
  clearFree(copy, len);
  char* tmp = oidc_sprintf("[%s]", json);
  clearFreeString(json);
  if(tmp==NULL) {
    return NULL;
  }
  return tmp;
}

char* listToJSONArray(list_t* list) {
  if(list==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  char* tmp = NULL;
  if(list->len >= 1) {
    char* json = oidc_sprintf("\"%s\"", list_at(list, 0)->val);
    size_t i;
    for(i=1; i<list->len; i++) {
      char* tmp = oidc_sprintf("%s, \"%s\"", json, list_at(list, i)->val);
      clearFreeString(json);
      if(tmp==NULL) {
        return NULL;
      }
      json = tmp;
    }
    tmp = oidc_sprintf("[%s]", json);
    clearFreeString(json);
  } else {
    tmp = oidc_strcopy("[]");
  }


  if(tmp==NULL) {
    return NULL;
  }
  return tmp;
}

list_t* delimitedStringToList(char* str, char delimiter) {
  if(str==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }

  char* copy = oidc_sprintf("%s", str);
  int len = strlen(copy);
  char* delim = oidc_sprintf("%c", delimiter);
  list_t* list = list_new();
  list->free = (void(*) (void*)) &clearFreeString;
  list->match = (int(*) (void*, void*)) &strequal;
  char* elem = strtok(copy, delim);
  while(elem!=NULL) {
    list_rpush(list, list_node_new(oidc_sprintf(elem)));
    elem = strtok(NULL, delim);
  }
  clearFreeString(delim);
  clearFree(copy, len);
  return list;
}

char* listToDelimitedString(list_t* list, char delimiter) {
  list_node_t* node = list_lpop(list);
  char* str = NULL;
  char* tmp = NULL;
  if(node==NULL) {
    str = oidc_sprintf("");
  } else {
    str = oidc_sprintf("%s", node->val);
    if(list->free) { list->free(node->val); }
    LIST_FREE(node);
  }
  while((node = list_lpop(list))!=NULL) {
    tmp = oidc_sprintf("%s%c%s", str, delimiter, node->val);
    clearFreeString(str);
    if(list->free) { list->free(node->val); }
    LIST_FREE(node);
    if(tmp==NULL) {
      return NULL;
    }
    str = tmp;
  }
  return str;
}

/** 
 * eliminates a character c if it is followed by character f
 */
char* strelimIfFollowed(char* str, char c, char f) {
  if(!isValid(str)) {
    return str;
  }
  size_t len = strlen(str);
  size_t i, j;
  for(i=0; i<len-1; i++) {
    if(str[i]==c && str[i+1]==f) {
      for (j=i; j<len-1; j++) {
        str[j]=str[j+1];   
      }
      str[j] = '\0';
    }
  }
  // syslog(LOG_AUTHPRIV|LOG_DEBUG, "In strelim eliminating '%c'; new string is '%s'", c, str);
  return str;
}

char* strelim(char str[], char c) {
  if(str==NULL) {
    return NULL;
  }
  size_t len = strlen(str);
  size_t i, j;
  for(i=0; i<len; i++) {
    if(str[i]==c) {
      for (j=i; j<len; j++) {
        str[j]=str[j+1];   
      }
    }
  }
  // syslog(LOG_AUTHPRIV|LOG_DEBUG, "In strelim eliminating '%c'; new string is '%s'", c, str);
  return str;
}

int listStringToArray(const char* str, char delimiter, char** arr) {
  size_t size = strCountChar(str, delimiter)+1; 
  if(arr==NULL) {
    return size;
  }
  char* arr_str = oidc_sprintf("%s", str); 
  char* orig = arr_str;
  int len = strlen(orig);
  if(arr_str[0]=='[') { arr_str++;  }
  if(arr_str[strlen(arr_str)-1]==']') { arr_str[strlen(arr_str)-1] = '\0'; }
  char* delim = oidc_sprintf("%c", delimiter);
  arr[0] = oidc_sprintf("%s", strtok(arr_str, delim));
  unsigned int i;
  for(i=1; i<size; i++) {
    arr[i] = oidc_sprintf("%s", strtok(NULL, delim));
  }
  clearFreeString(delim);
  clearFree(orig, len);
  return i;
}

size_t strCountChar(const char* s, char c) {
  int i;
  for (i=0; s[i]; s[i]==c ? i++ : *s++);
  return i;
}

/**
 * @brief asserts that the oidc directory exists
 */
void assertOidcDirExists() {
  char* dir = NULL;
  if((dir = getOidcDir())==NULL) {
    printError("Error: oidc-dir does not exist. Run make to create it.\n");
    exit(EXIT_FAILURE);
  }
  clearFreeString(dir);
}

int printError(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* colored = oidc_sprintf("%s%s%s", C_ERROR, fmt, C_RESET);
  int ret = vfprintf(stderr, colored, args);
  clearFreeString(colored);
  return ret;
}

unsigned char* decryptFileContent(const char* fileContent, const char* password) {
  if(fileContent==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  int len = strlen(fileContent);
  char* fileText = calloc(sizeof(char), len+1);
  strcpy(fileText, fileContent);
  unsigned long cipher_len = atoi(strtok(fileText, ":"));
  char* salt_hex = strtok(NULL, ":");
  char* nonce_hex = strtok(NULL, ":");
  char* cipher = strtok(NULL, ":");
  unsigned char* decrypted = crypt_decrypt(cipher, cipher_len, password, nonce_hex, salt_hex);
  clearFree(fileText, len);
  return decrypted;
}

list_t* intersectLists(list_t* a, list_t* b) {
  list_t* l = list_new();
  l->free = (void(*) (void*))  &clearFreeString;
  l->match = (int(*) (void*, void*)) &strequal;
  list_node_t *node;
  list_iterator_t *it = list_iterator_new(a, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    list_node_t* n = list_find(b, node->val);
    if(n) {
      list_rpush(l, list_node_new(oidc_strcopy(n->val)));
    }
  }
  list_iterator_destroy(it);
  return l;
}

int strequal(const char* a, const char* b) {
  return strcmp(a, b)==0 ? 1 : 0;
}

int strcaseequal(const char* a, const char* b) {
  return strcasecmp(a, b)==0 ? 1 : 0;
}

char* combineError(const char* error, const char* error_description) {
  if(!isValid(error) && !isValid(error_description)) {
    return NULL;
  }
  if(!isValid(error_description)) {
    return oidc_strcopy(error);
  }
  return oidc_sprintf("%s: %s", error, error_description);
}

char* escapeCharInStr(const char* str, char c) {
  if(str==NULL) {
    return NULL;
  }
  char* s = oidc_strcopy(str); 
  const char* pos = s;
  unsigned int rel = pos-s;
  while(rel<strlen(s) && (pos=strchr(s+rel, c))!=NULL) {
    rel = pos - s;
    s = realloc(s, strlen(s)+1+1);
    memmove(s+rel+1, s+rel, strlen(s+rel)+1);
    s[rel]='\\';
    rel+=2;
  }
  return s;
}
