#ifndef OIDC_API_H
#define OIDC_API_H

#include <time.h>

struct connection* initTokenSocket() ;
void closeTokenSocket(struct connection* con) ;
int tryAccept(struct connection* con, time_t timeout_s) ;
int communicate(struct connection* con) ;

#endif // OIDC_API_H
