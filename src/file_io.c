#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

char* possibleLocations[] = {"~/.config/oidc/", "~/.oidc/"};

/** @fn char* readFile(const char* path)
 * @brief reads a file and returns a poitner to the content
 * @param path the file to be read
 * @return a pointer to the file content. Has to be freed after usage.
 */
char* readFile(const char* path) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Reading file: %s", path);
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( path, "rb" );
  if( !fp ) {
    syslog(LOG_AUTHPRIV|LOG_NOTICE, "%m\n");
    return NULL;
  }

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  buffer = calloc( 1, lSize+1 );
  if( !buffer ){
    fclose(fp);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "memory alloc failed in function readFile '%s': %m\n", path);
    exit(EXIT_FAILURE);
  }

  if( 1!=fread( buffer , lSize, 1 , fp) ) {
    fclose(fp);
    free(buffer);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "entire read failed in function readFile '%s': %m\n", path);
    exit(EXIT_FAILURE);
  }
  fclose(fp);
  return buffer;
}

/** @fn void writeToFile(const char* path, const char* text)
 * @brief writes text to a file
 * @note \p text has to be nullterminated and must not contain nullbytes. If you
 * want to write a string containing nullbytes use \f writeBufferToFile instead
 * @param path the file to be written
 * @param text the nullterminated text to be written
 */
void writeToFile(const char* path, const char* text) {
  FILE *f = fopen(path, "w");
  if (f == NULL)
  {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error opening file '%s' in function writeToFile().\n", path);
    exit(EXIT_FAILURE);
  }
  fprintf(f, "%s", text);

  fclose(f);
}

int dirExists(const char* path) {
  DIR* dir = opendir(path);
  if (dir) {/* Directory exists. */
    closedir(dir);
    return 1;
  } else if (ENOENT == errno) { /* Directory does not exist. */
    return 0;
  } else { /* opendir() failed for some other reason. */
    syslog(LOG_AUTHPRIV|LOG_ALERT, "opendir: %m");
    exit(EXIT_FAILURE);
    return -1;
  }
}

char* getOidcDir() {
  char* home = getenv("HOME");
  unsigned int i;
  for(i=0; i<sizeof(possibleLocations)/sizeof(*possibleLocations); i++) {
    char* path = calloc(sizeof(char), strlen(home)+strlen(possibleLocations[i]+1)+1);
    sprintf(path, "%s%s", home, possibleLocations[i]+1);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Checking if dir '%s' exists.", path);
    if(dirExists(path)>0) 
      return path;
    free(path);
  }
  return NULL;
}

int fileDoesExist(const char* path) {
  return access(path, F_OK)==0 ? 1 : 0;
}

int oidcFileDoesExist(const char* filename) {
  char* oidc_dir = getOidcDir();
  char* path = calloc(sizeof(char), strlen(filename)+strlen(oidc_dir)+1);
  sprintf(path, "%s%s", oidc_dir, filename);
  free(oidc_dir);
  int b = fileDoesExist(path);
  free(path);
  return b;
}

void writeOidcFile(const char* filename, const char* text) {
  char* oidc_dir = getOidcDir();
  char* path = calloc(sizeof(char), strlen(filename)+strlen(oidc_dir)+1);
  sprintf(path, "%s%s", oidc_dir, filename);
  free(oidc_dir);
  writeToFile(path, text);
  free(path);
}

char* readOidcFile(const char* filename) {
  char* oidc_dir = getOidcDir();
  char* path = calloc(sizeof(char), strlen(filename)+strlen(oidc_dir)+1);
  sprintf(path, "%s%s", oidc_dir, filename);
  free(oidc_dir);
  char* c = readFile(path);
  free(path);
  return c;
}

int removeFile(const char* path) {
  return unlink(path);
}

int removeOidcFile(const char* filename) {
char* oidc_dir = getOidcDir();
  char* path = calloc(sizeof(char), strlen(filename)+strlen(oidc_dir)+1);
  sprintf(path, "%s%s", oidc_dir, filename);
  free(oidc_dir);
  int r = removeFile(path);
  free(path);
  return r;
}

