#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

/** @fn char* readFile(const char* filename)
 * @brief reads a file and returns a poitner to the content
 * @param filename the file to be read
 * @return a pointer to the file content. Has to be freed after usage.
 */
char* readFile(const char* filename) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Reading file: %s", filename);
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( filename, "rb" );
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
    syslog(LOG_AUTHPRIV|LOG_EMERG, "memory alloc failed in function readFile '%s': %m\n", filename);
    exit(EXIT_FAILURE);
  }

  if( 1!=fread( buffer , lSize, 1 , fp) ) {
    fclose(fp);
    free(buffer);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "entire read failed in function readFile '%s': %m\n", filename);
    exit(EXIT_FAILURE);
  }
  fclose(fp);
  return buffer;
}

/** @fn void writeToFile(const char* filename, const char* text)
 * @brief writes text to a file
 * @note \p text has to be nullterminated and must not contain nullbytes. If you
 * want to write a string containing nullbytes use \f writeBufferToFile instead
 * @param filename the file to be written
 * @param text the nullterminated text to be written
 */
void writeToFile(const char* filename, const char* text) {
  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error opening file '%s' in function writeToFile().\n", filename);
    exit(EXIT_FAILURE);
  }
  fprintf(f, "%s", text);

  fclose(f);
}

/** @fn void writeBufferToFile(const char* filename, const char* text, int len)
 * @brief writes a buffer to file. The buffer may contains null bytes.
 * @param filename the file to be written
 * @text the text to be written
 * @len the number of bytes to be written
 */
void writeBufferToFile(const char* filename, const char* text, int len) {
  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    syslog(LOG_AUTHPRIV|LOG_ALERT, "Error opening file '%s' in function writeToFile().\n", filename);
    exit(EXIT_FAILURE);
  }
  fwrite(text, len, 1, f);

  fclose(f);
}

