#ifndef SERVICE_H
#define SERVICE_H

int provider = 0;
int refresh = 0;


void parseOpt(int argc, char* const* argv) ;
int tryRefreshFlow() ;
int tryPasswordFlow() ;

#endif //SERVICE_H
