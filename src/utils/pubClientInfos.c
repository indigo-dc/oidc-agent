#include "pubClientInfos.h"

#include <string.h>

#include "account/issuer_helper.h"
#include "defines/msys.h"
#include "defines/settings.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"

void secFreePubClientInfos(struct pubClientInfos* p) {
  if (p == NULL) {
    return;
  }
  secFree(p->client_id);
  secFree(p->client_secret);
  secFree(p->scope);
  secFree(p);
}

struct pubClientInfos* _getPubClientInfosFromList(list_t*     lines,
                                                  const char* issuer) {
  if (lines == NULL) {
    return NULL;
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(lines, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* client = strtok(node->val, "@");
    char* iss    = strtok(NULL, "@");
    char* scope  = strtok(NULL, "@");
    // logger(DEBUG, "Found public client for '%s'", iss);
    if (compIssuerUrls(issuer, iss)) {
      char*                  client_id     = strtok(client, ":");
      char*                  client_secret = strtok(NULL, ":");
      struct pubClientInfos* infos = secAlloc(sizeof(struct pubClientInfos));
      infos->client_id             = oidc_strcopy(client_id);
      infos->client_secret         = oidc_strcopy(client_secret);
      infos->scope                 = oidc_strcopy(scope);
      list_iterator_destroy(it);
      return infos;
    }
  }
  list_iterator_destroy(it);
  return NULL;
}

struct pubClientInfos* getPubClientInfos(const char* issuer) {
  list_t* pubClientLines = getLinesFromFileWithoutComments(
#ifdef ANY_MSYS
      ETC_PUBCLIENTS_CONFIG_FILE()
#else
      ETC_PUBCLIENTS_CONFIG_FILE
#endif
  );
  struct pubClientInfos* infos =
      _getPubClientInfosFromList(pubClientLines, issuer);
  secFreeList(pubClientLines);
  if (infos != NULL) {
    return infos;
  }
  pubClientLines = getLinesFromOidcFileWithoutComments(PUBCLIENTS_FILENAME);
  infos          = _getPubClientInfosFromList(pubClientLines, issuer);
  secFreeList(pubClientLines);
  return infos;
}

list_t* defaultRedirectURIs() {
  list_t* redirect_uris =
      createList(0, "http://localhost:8080", "http://localhost:4242",
                 "http://localhost:43985", NULL);
  redirect_uris->match = (matchFunction)strequal;
  return redirect_uris;
}
