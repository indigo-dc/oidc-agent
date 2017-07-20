#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

char* readFile(const char* filename) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Reading file: %s", filename);
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( filename, "rb" );
  if( !fp ) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%m\n");
    exit(EXIT_FAILURE);
  }

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  buffer = calloc( 1, lSize+1 );
  if( !buffer ){
    fclose(fp);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "memory alloc failed in function readFile '%s'.\n", filename);
    exit(EXIT_FAILURE);
  }

  if( 1!=fread( buffer , lSize, 1 , fp) ) {
    fclose(fp);
    free(buffer);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "entire read failed in function readFile '%s'.\n", filename);
    exit(EXIT_FAILURE);
  }
  fclose(fp);
  return buffer;
}


void writeToFile(const char* filename, const char* text) {
  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    fprintf(stderr, "Error opening file! %s\n", filename);
    syslog(LOG_AUTHPRIV|LOG_EMERG, "Error opening file '%s' in function writeToFile().\n", filename);
    exit(EXIT_FAILURE);
  }
  fprintf(f, "%s", text);

  fclose(f);
}

