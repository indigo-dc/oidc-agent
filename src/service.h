#ifndef SERVICE_H
#define SERVICE_H

#include <stddef.h>

unsigned int provider = 0;
char* cwd = NULL;

void parseOpt(int argc, char* const* argv) ;
int tryRefreshFlow() ;
int tryPasswordFlow() ;
int getAccessToken() ;
int test();
char* getUserInput(char* prompt) ;
void runPassprompt() ;

#endif //SERVICE_H
