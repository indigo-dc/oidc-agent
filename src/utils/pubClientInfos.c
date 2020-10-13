#include "pubClientInfos.h"

#include "account/issuer_helper.h"
#include "defines/settings.h"
#include "utils/file_io/file_io.h"
#include "utils/listUtils.h"
#include "utils/memory.h"
#include "utils/stringUtils.h"

#include <string.h>

void secFreePubClientInfos(struct pubClientInfos p) {
  secFree(p.client_id);
  secFree(p.client_secret);
  secFree(p.scope);
}

struct pubClientInfos getPubClientInfos(const char* issuer) {
  struct pubClientInfos ret = {0, 0, 0};
  list_t* pubClientLines    = getLinesFromFile(ETC_PUBCLIENTS_CONFIG_FILE);
  if (pubClientLines == NULL) {
    return ret;
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(pubClientLines, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* client = strtok(node->val, "@");
    char* iss    = strtok(NULL, "@");
    char* scope  = strtok(NULL, "@");
    // logger(DEBUG, "Found public client for '%s'", iss);
    if (compIssuerUrls(issuer, iss)) {
      char* client_id     = strtok(client, ":");
      char* client_secret = strtok(NULL, ":");
      ret.client_id       = oidc_strcopy(client_id);
      ret.client_secret   = oidc_strcopy(client_secret);
      ret.scope           = oidc_strcopy(scope);
      break;
    }
  }
  list_iterator_destroy(it);
  list_destroy(pubClientLines);
  return ret;
}

list_t* defaultRedirectURIs() {
  list_t* redirect_uris =
      createList(0, "http://localhost:8080", "http://localhost:4242",
                 "http://localhost:43985", NULL);
  redirect_uris->match = (matchFunction)strequal;
  return redirect_uris;
}
