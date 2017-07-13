#ifndef OIDC_H
#define OIDC_H

int refreshToken(int povider_i) ;
int passwordFlow(int provider_i, const char* password) ;
#endif //OIDC_H
