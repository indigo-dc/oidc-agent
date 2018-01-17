#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>

#include "oidc_utilities.h"
#include "oidc_error.h"
#include "file_io.h"

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



long random_at_most(long max) {
  unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned long) max + 1,
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
  return oidc_sprintf("http://localhost:%d", port);
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
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "In strelim eliminating '%c'; new string is '%s'", c, str);
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
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "In strelim eliminating '%c'; new string is '%s'", c, str);
  return str;
}

int listStringToArray(const char* str, char delimiter, char** arr) {
  size_t size = strCountChar(str, delimiter)+1; 
  if(arr==NULL) {
    return size;
  }
  char* arr_str = oidc_sprintf("%s", str); 
  char* orig = arr_str;
  if(arr_str[0]=='[') { arr_str++;  }
  if(arr_str[strlen(arr_str)-1]==']') { arr_str[strlen(arr_str)-1] = '\0'; }
  char* delim = oidc_sprintf("%c", delimiter);
  arr[0] = oidc_sprintf("%s", strtok(arr_str, delim));
  unsigned int i;
  for(i=1; i<size; i++) {
    arr[i] = oidc_sprintf("%s", strtok(NULL, delim));
  }
  clearFreeString(delim);
  clearFreeString(orig);
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
    fprintf(stderr, "Error: oidc-dir does not exist. Run make to create it.\n");
    exit(EXIT_FAILURE);
  }
  clearFreeString(dir);
}

