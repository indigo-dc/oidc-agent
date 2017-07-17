#include <stdio.h>
#include <stdlib.h>

#include "logger.h"

char* readFile(const char* filename) {
  logging(DEBUG, "Reading file: %s", filename);
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen ( filename, "rb" );
  if( !fp ) perror(filename),exit(EXIT_FAILURE);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  buffer = calloc( 1, lSize+1 );
  if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  if( 1!=fread( buffer , lSize, 1 , fp) )
    fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

  fclose(fp);
  return buffer;
}


void writeToFile(const char* filename, const char* text) {
  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    fprintf(stderr, "Error opening file! %s\n", filename);
    exit(EXIT_FAILURE);
  }
  fprintf(f, "%s", text);

  fclose(f);
}

