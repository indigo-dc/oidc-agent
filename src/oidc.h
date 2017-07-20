#ifndef OIDC_H
#define OIDC_H

int getAccessToken() ;
int tryRefreshFlow() ;
int tryPasswordFlow() ;
int refreshFlow(int povider_i) ;
int passwordFlow(int provider_i, const char* password) ;

#endif //OIDC_H
