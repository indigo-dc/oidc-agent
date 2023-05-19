#define _GNU_SOURCE
#include "issuer_helper.h"

#include <string.h>

#include "defines/agent_values.h"
#include "defines/msys.h"
#include "defines/oidc_values.h"
#include "utils/config/issuerConfig.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

char* getUsableGrantTypes(const struct oidc_account* account, list_t* flows) {
  const char* supported = account_getGrantTypesSupported(account);
  if (supported == NULL || flows == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* supp   = JSONArrayStringToList(supported);
  list_t* wanted = list_new();
  wanted->match  = (matchFunction)strequal;
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
  logger(DEBUG, "wanted grant types are: %s", wanted_str);
  secFree(wanted_str);
  logger(DEBUG, "daeSetByUser is: %d",
         issuer_getDeviceAuthorizationEndpointIsSetByUser(
             account_getIssuer(account)));
  list_t* usable = intersectLists(wanted, supp);
  secFreeList(supp);
  secFreeList(wanted);
  if (account_getIssuer(account)
          ? issuer_getDeviceAuthorizationEndpointIsSetByUser(
                account_getIssuer(account))
          : 0 && findInList(usable, OIDC_GRANTTYPE_DEVICE) ==
                     NULL) {  // Force device grant type when device
    // authorization endpoint set by user
    logger(DEBUG, "Forcing device grant type");
    list_rpush(usable, list_node_new(oidc_strcopy(OIDC_GRANTTYPE_DEVICE)));
  }
  char* str = listToJSONArrayString(usable);
  secFreeList(usable);
  logger(DEBUG, "usable grant types are: %s", str);
  return str;
}

char* getUsableResponseTypes(const struct oidc_account* account,
                             list_t*                    flows) {
  list_t* supp =
      JSONArrayStringToList(account_getResponseTypesSupported(account));

  list_t* wanted = list_new();
  wanted->match  = (matchFunction)strequal;
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
  secFreeList(supp);
  secFreeList(wanted);
  char* str = listToJSONArrayString(usable);
  secFreeList(usable);
  logger(DEBUG, "usable response types are: %s", str);
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
    return strequal(a, b);
  }
  if (b_len == a_len - 1) {
    const char* t     = a;
    size_t      t_len = a_len;
    a                 = b;
    a_len             = b_len;
    b                 = t;
    b_len             = t_len;
  }
  if (a_len == b_len - 1 && b[b_len - 1] == '/') {
    return strncmp(a, b, a_len) == 0 ? 1 : 0;
  }
  return 0;
}

void printIssuerHelp(const char* url) {
  const struct issuerConfig* c = getIssuerConfig(url);
  if (c == NULL) {
    printStdout("Unfortunately no contact information were found for "
                "issuer '%s'\n",
                url);
    return;
  }
  if (strValid(c->manual_register)) {
    printStdout("You can try to register a client manually at '%s'\n",
                c->manual_register);
  }
  if (strValid(c->contact)) {
    printStdout("You can contact the OpenID Provider at '%s'\n", c->contact);
  }
}

size_t getFavIssuer(const struct oidc_account* account, list_t* suggestable) {
  if (strValid(account_getIssuerUrl(
          account))) {  // if issuer already set suggest this one
    for (size_t i = 0; i < suggestable->len; i++) {
      list_node_t* node = list_at(suggestable, i);
      if (compIssuerUrls(
              node->val,
              account_getIssuerUrl(account))) {  // if the short name is a
                                                 // substring of the issuer
        // it's likely that this is the fav issuer
        return i;
      }
    }
    list_rpush(suggestable,
               list_node_new(suggestable->free == _secFree
                                 ? oidc_strcopy(account_getIssuerUrl(account))
                                 : account_getIssuerUrl(account)));
    return suggestable->len - 1;
  }

  for (size_t i = 0; i < suggestable->len; i++) {
    list_node_t* node = list_at(suggestable, i);
    if (strSubStringCase(
            node->val,
            account_getName(account))) {  // if the short name is a substring
                                          // of the issuer it's likely that
                                          // this is the fav issuer
      return i;
    }
  }
  return 0;
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
