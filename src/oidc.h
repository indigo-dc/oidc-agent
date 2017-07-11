#ifndef OIDC_H
#define OIDC_H

char* refreshToken(int povider_i) ;
char* passwordFlow(int provider_i, const char* password) ;
#endif //OIDC_H
