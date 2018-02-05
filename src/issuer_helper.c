#define _GNU_SOURCE
#include "issuer_helper.h"
#include "file_io.h"

#include <string.h>
#include <syslog.h>


char* getUsableGrantTypes(const char* supported, int usePasswordGrantType) {
  if(supported==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  list_t* supp = JSONArrayToList(supported);
  list_t* wanted = delimitedStringToList(usePasswordGrantType ? "refresh_token authorization_code password" : "refresh_token authorization_code", ' ');
  list_t* usable = intersectLists(wanted, supp);
  list_destroy(supp);
  list_destroy(wanted);
  char* str = listToJSONArray(usable);
  list_destroy(usable);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "usable grant types are: %s", str);
  return str;
}

char* getUsableResponseTypes(struct oidc_account account, int usePasswordGrantType) {
  list_t* supp = JSONArrayToList(account_getResponseTypesSupported(account));
  list_t* wanted = delimitedStringToList(usePasswordGrantType && strstr(account_getGrantTypesSupported(account), "password") ? "code token" : "code", ' ');
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "usable response types are: %s", strstr(account_getGrantTypesSupported(account), "password") ? "code token" : "code");
  list_t* usable = intersectLists(wanted, supp);
  list_destroy(supp);
  list_destroy(wanted);
  char* str = listToJSONArray(usable);
  list_destroy(usable);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "usable response types are: %s", str);
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
  size_t a_len = strlen(a);
  size_t b_len = strlen(b);
  if(a_len==b_len) {
    return strcmp(a, b)==0 ? 1 : 0;
  }
  if(b_len==a_len-1) {
    const char* t = a; size_t t_len = a_len;
    a = b; a_len = b_len;
    b = t; b_len = t_len;
  }
  if(a_len==b_len-1){
    if(b[b_len-1]=='/') {
      return strncmp(a, b, a_len)==0 ? 1 : 0;
    }
    return 0;
  }
  return 0;

}

void printIssuerHelp(const char* url) {
  char* fileContent = readFile(ETC_ISSUER_CONFIG_FILE);
  size_t len = strlen(fileContent);
  char* elem = strtok(fileContent, "\n");
  while(elem!=NULL) {
    char* space = strchr(elem, ' ');
    if(space) {
      *space = '\0';
    }
    if(compIssuerUrls(url, elem)) {
      if(space) {
        char* reg_uri = space+1;
        space = strchr(reg_uri, ' ');
        char* contact = NULL;
        if(space) {
          *space = '\0';
          contact = space+1;
        }
        if(isValid(reg_uri)) {
          printf("You can try to register a client manually at '%s'\n", reg_uri);
        }
        if(isValid(contact)) {
          printf("You can contact the OpenID Provider at '%s'\n", contact);
        }
      } else {
        printf("Unfortunately no contact information were found for issuer '%s'\n", url);
      }
      break;
    }
    elem = strtok(NULL, "\n");
  }
  clearFree(fileContent, len);

}

list_t* getSuggestableIssuers() {
  list_t* issuers = list_new();
  issuers->free = (void(*) (void*)) &clearFreeString;
  issuers->match = (int(*) (void*, void*)) &compIssuerUrls;

  char* fileContent = readOidcFile(ISSUER_CONFIG_FILENAME);
  if(fileContent) {
    size_t len = strlen(fileContent);
    char* elem = strtok(fileContent, "\n");
    while(elem!=NULL) {
      char* space = strchr(elem, ' ');
      if(space) {
        *space = '\0';
      }
      list_rpush(issuers, list_node_new(oidc_sprintf(elem)));
      elem = strtok(NULL, "\n");
    }
    clearFree(fileContent, len);
  }

  fileContent = readFile(ETC_ISSUER_CONFIG_FILE);
  if(fileContent) {
    size_t len = strlen(fileContent);
    char* elem = strtok(fileContent, "\n");
    while(elem!=NULL) {
      char* space = strchr(elem, ' ');
      if(space) {
        *space = '\0';
      }
      if(list_find(issuers, elem)==NULL) {
        list_rpush(issuers, list_node_new(oidc_sprintf(elem)));
      }
      elem = strtok(NULL, "\n");
    }
    clearFree(fileContent, len);

  }

  return issuers;
}

char* getFavIssuer(struct oidc_account* account, list_t* suggastable) {
  if(isValid(account_getIssuerUrl(*account))) {
    return account_getIssuerUrl(*account);
  }
  list_node_t* node;
  list_iterator_t* it = list_iterator_new(suggastable, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    if(strcasestr(node->val, account_getName(*account))) { // if the short name is a substring of the issuer it's likely that this is the fav issuer
      list_iterator_destroy(it);
      return node->val;
    }
  }
  list_iterator_destroy(it);
  return list_at(suggastable, 0) ? list_at(suggastable, 0)->val: "";
} 

void printSuggestIssuer(list_t* suggastable) {
  if(suggastable==NULL) {
    return;
  }
  size_t i;
  for(i=0; i<suggastable->len; i++) {// printed indices starts at 1 for non nerd
    printf(C_PROMPT "[%lu] %s\n" C_RESET, i+1, list_at(suggastable, i)->val); 
  }
}
