#ifndef TOKEN_IPC_H
#define TOKEN_IPC_H

#include <time.h>

struct connection* initTokenSocket() ;
void closeTokenSocket(struct connection* con) ;
int tryAccept(struct connection* con, time_t timeout_s) ;
void communicate(struct connection* con) ;

#endif // TOKEN_IPC_H
