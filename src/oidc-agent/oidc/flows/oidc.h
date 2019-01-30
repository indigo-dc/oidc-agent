#ifndef OIDC_H
#define OIDC_H

#include "account/account.h"
#include "ipc/pipe.h"

char* generatePostData(char* k1, char* v1, ...);
char* generatePostDataFromList(list_t* list);
char* parseTokenResponse(const char* res, struct oidc_account* a,
                         int saveAccessToken, struct ipcPipe pipes);
char* parseTokenResponseCallbacks(
    const char* res, struct oidc_account* a, int saveAccessToken,
    void (*errorHandling)(const char*, const char*), struct ipcPipe pipes);

#endif  // OIDC_H
