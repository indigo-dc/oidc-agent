#ifndef SERVICE_H
#define SERVICE_H

int provider = 0;
int refresh = 0;
int single_run=0;


void parseOpt(int argc, char* const* argv) ;
int tryRefreshFlow() ;
int tryPasswordFlow() ;
void writeToFile(const char* filename, const char* text) ;
void test();

#endif //SERVICE_H
