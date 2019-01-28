#define _GNU_SOURCE
#include "issuer_helper.h"

#include "ipc/ipc_values.h"
#include "oidc-agent/oidc/values.h"
#include "settings.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/listUtils.h"
#include "utils/pass.h"

#include <string.h>
#include <syslog.h>

char* getUsableGrantTypes(const struct oidc_account* account, list_t* flows) {
  const char* supported = account_getGrantTypesSupported(account);
  if (supported == NULL || flows == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* supp   = JSONArrayStringToList(supported);
  list_t* wanted = list_new();
  wanted->match  = (int (*)(void*, void*))strequal;
  list_rpush(wanted, list_node_new(OIDC_GRANTTYPE_REFRESH));
  list_node_t*     node;
  list_iterator_t* it       = list_iterator_new(flows, LIST_HEAD);
  int              code     = 0;
  int              password = 0;
  while ((node = list_iterator_next(it))) {
    if (strcaseequal(node->val, FLOW_VALUE_PASSWORD) && !code) {
      list_rpush(wanted, list_node_new(OIDC_GRANTTYPE_PASSWORD));
      password = 1;
    }
    if (strcaseequal(node->val, FLOW_VALUE_CODE) && !password) {
      list_rpush(wanted, list_node_new(OIDC_GRANTTYPE_AUTHCODE));
      code = 1;
    }
    if (strcaseequal(node->val, FLOW_VALUE_DEVICE)) {
      list_rpush(wanted, list_node_new(OIDC_GRANTTYPE_DEVICE));
    }
  }
  list_iterator_destroy(it);
  char* wanted_str = listToJSONArrayString(wanted);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "wanted grant types are: %s", wanted_str);
  secFree(wanted_str);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "daeSetByUser is: %d",
         issuer_getDeviceAuthorizationEndpointIsSetByUser(
             account_getIssuer(account)));
  list_t* usable = intersectLists(wanted, supp);
  list_destroy(supp);
  list_destroy(wanted);
  if (account_getIssuer(account)
          ? issuer_getDeviceAuthorizationEndpointIsSetByUser(
                account_getIssuer(account))
          : 0 && findInList(usable, OIDC_GRANTTYPE_DEVICE) ==
                     NULL) {  // Force device grant type when device
    // authorization endpoint set by user
    syslog(LOG_AUTHPRIV | LOG_DEBUG, "Forcing device grant type");
    list_rpush(usable, list_node_new(oidc_strcopy(OIDC_GRANTTYPE_DEVICE)));
  }
  char* str = listToJSONArrayString(usable);
  list_destroy(usable);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "usable grant types are: %s", str);
  return str;
}

char* getUsableResponseTypes(const struct oidc_account* account,
                             list_t*                    flows) {
  list_t* supp =
      JSONArrayStringToList(account_getResponseTypesSupported(account));

  list_t* wanted = list_new();
  wanted->match  = (int (*)(void*, void*))strequal;
  list_node_t* node;
  if (flows) {
    list_iterator_t* it    = list_iterator_new(flows, LIST_HEAD);
    int              code  = 0;
    int              token = 0;
    while ((node = list_iterator_next(it))) {
      if (strcaseequal(node->val, FLOW_VALUE_PASSWORD) && !token) {
        list_rpush(wanted, list_node_new(OIDC_RESPONSETYPE_TOKEN));
        token = 1;
      }
      if (strcaseequal(node->val, FLOW_VALUE_CODE) && !token && !code) {
        list_rpush(wanted, list_node_new(OIDC_RESPONSETYPE_CODE));
        code = 1;
      }
      if (strcaseequal(node->val, FLOW_VALUE_DEVICE) && !token) {
        list_rpush(wanted, list_node_new(OIDC_RESPONSETYPE_TOKEN));
        token = 1;
      }
    }
    list_iterator_destroy(it);
  }
  list_t* usable = intersectLists(wanted, supp);
  list_destroy(supp);
  list_destroy(wanted);
  char* str = listToJSONArrayString(usable);
  list_destroy(usable);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "usable response types are: %s", str);
  return str;
}

/**
 * Compares two issuer urls.
 * Two issuer urls are defined equal if they are:
 *  - completely equal (strcmp==0) or
 *  - one url misses the trailing /
 * @return 1 if equal, 0 if not
 */
int compIssuerUrls(const char* a, const char* b) {
  if (a == NULL || b == NULL) {
    oidc_setArgNullFuncError(__func__);
    return 0;
  }
  size_t a_len = strlen(a);
  size_t b_len = strlen(b);
  if (a_len == b_len) {
    return strcmp(a, b) == 0 ? 1 : 0;
  }
  if (b_len == a_len - 1) {
    const char* t     = a;
    size_t      t_len = a_len;
    a                 = b;
    a_len             = b_len;
    b                 = t;
    b_len             = t_len;
  }
  if (a_len == b_len - 1) {
    if (b[b_len - 1] == '/') {
      return strncmp(a, b, a_len) == 0 ? 1 : 0;
    }
    return 0;
  }
  return 0;
}

void printIssuerHelp(const char* url) {
  if (!fileDoesExist(ETC_ISSUER_CONFIG_FILE)) {
    return;
  }
  char* fileContent = readFile(ETC_ISSUER_CONFIG_FILE);
  char* elem        = strtok(fileContent, "\n");
  while (elem != NULL) {
    char* space = strchr(elem, ' ');
    if (space) {
      *space = '\0';
    }
    if (compIssuerUrls(url, elem)) {
      if (space) {
        char* reg_uri = space + 1;
        space         = strchr(reg_uri, ' ');
        char* contact = NULL;
        if (space) {
          *space  = '\0';
          contact = space + 1;
        }
        if (strValid(reg_uri)) {
          printf("You can try to register a client manually at '%s'\n",
                 reg_uri);
        }
        if (strValid(contact)) {
          printf("You can contact the OpenID Provider at '%s'\n", contact);
        }
      } else {
        printf(
            "Unfortunately no contact information were found for issuer '%s'\n",
            url);
      }
      break;
    }
    elem = strtok(NULL, "\n");
  }
  secFree(fileContent);
}

list_t* getSuggestableIssuers() {
  list_t* issuers = list_new();
  issuers->free   = (void (*)(void*)) & _secFree;
  issuers->match  = (int (*)(void*, void*)) & compIssuerUrls;

  char* fileContent = readOidcFile(ISSUER_CONFIG_FILENAME);
  if (fileContent) {
    char* elem = strtok(fileContent, "\n");
    while (elem != NULL) {
      char* space = strchr(elem, ' ');
      if (space) {
        *space = '\0';
      }
      if (findInList(issuers, elem) == NULL) {
        list_rpush(issuers, list_node_new(oidc_sprintf(elem)));
      }
      elem = strtok(NULL, "\n");
    }
    secFree(fileContent);
  }

  fileContent = readFile(ETC_ISSUER_CONFIG_FILE);
  if (fileContent) {
    char* elem = strtok(fileContent, "\n");
    while (elem != NULL) {
      char* space = strchr(elem, ' ');
      if (space) {
        *space = '\0';
      }
      if (findInList(issuers, elem) == NULL) {
        list_rpush(issuers, list_node_new(oidc_sprintf(elem)));
      }
      elem = strtok(NULL, "\n");
    }
    secFree(fileContent);
  }

  return issuers;
}

char* getFavIssuer(const struct oidc_account* account, list_t* suggastable) {
  if (strValid(account_getIssuerUrl(account))) {
    return account_getIssuerUrl(account);
  }
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(suggastable, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    if (strSubStringCase(
            node->val,
            account_getName(
                account))) {  // if the short name is a substring of the issuer
                              // it's likely that this is the fav issuer
      list_iterator_destroy(it);
      return node->val;
    }
  }
  list_iterator_destroy(it);
  return list_at(suggastable, 0) ? list_at(suggastable, 0)->val : "";
}

void printSuggestIssuer(list_t* suggastable) {
  if (suggastable == NULL) {
    return;
  }
  size_t i;
  for (i = 0; i < suggastable->len;
       i++) {  // printed indices starts at 1 for non nerd
    printPrompt("[%lu] %s\n", i + 1, (char*)list_at(suggastable, i)->val);
  }
}
