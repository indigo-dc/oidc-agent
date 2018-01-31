#define _GNU_SOURCE
#include "issuer_helper.h"
#include "file_io.h"

#include <string.h>

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

list_t* getSuggestableIssuers() {
  list_t* issuers = list_new();
  issuers->free = (void(*) (void*)) &clearFreeString;
  issuers->match = (int(*) (void*, void*)) &compIssuerUrls;

  char* fileContent = readOidcFile(ISSUER_CONFIG_FILENAME);
  size_t len = strlen(fileContent);
  char* elem = strtok(fileContent, "\n");
  while(elem!=NULL) {
    list_rpush(issuers, list_node_new(oidc_sprintf(elem)));
    elem = strtok(NULL, "\n");
  }
  clearFree(fileContent, len);

  fileContent = readFile(ETC_ISSUER_CONFIG_FILE);
  len = strlen(fileContent);
  elem = strtok(fileContent, "\n");
  while(elem!=NULL) {
    if(list_find(issuers, elem)==NULL) {
    list_rpush(issuers, list_node_new(oidc_sprintf(elem)));
    }
    elem = strtok(NULL, "\n");
  }
  clearFree(fileContent, len);

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
