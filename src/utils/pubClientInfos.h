#ifndef PUBCLIENT_INFOS_H
#define PUBCLIENT_INFOS_H

#include "list/list.h"

struct pubClientInfos {
  char* client_id;
  char* client_secret;
  char* scope;
};

void                  secFreePubClientInfos(struct pubClientInfos p);
struct pubClientInfos getPubClientInfos(const char* issuer);
list_t*               defaultRedirectURIs();

#endif /* PUBCLIENT_INFOS_H */
