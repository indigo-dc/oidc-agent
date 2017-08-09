#include <stdio.h>
#include <string.h>

#include <syslog.h>

#include "provider.h"
#include "file_io.h"
#include "crypt.h"
#include "oidc_array.h"

struct oidc_provider* addProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider) {
  unsigned int i;
  for (i=0; i<*size; i++) {
    char* json = providerToJSON(*(p+i));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Provider: %s", json);
    free(json);
  }
  p= arr_addElement(p, size, sizeof(*p), &provider);    
  for (i=0; i<*size; i++) {
    char* json = providerToJSON(*(p+i));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Provider: %s", json);
    free(json);
  }
  return p;
}

/** @fn int provider_comparator(const void* v1, const void* v2)
 * @brief compares two providers by their name. Can be used for sorting.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
int provider_comparator(const void *v1, const void *v2) {
  const struct oidc_provider *p1 = (struct oidc_provider *)v1;
  const struct oidc_provider *p2 = (struct oidc_provider *)v2;
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p1 is %p", p1);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p2 is %p", p2);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p1name is %p", provider_getName(*p1));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p2name is %p", provider_getName(*p2));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p1name is %s", provider_getName(*p1));
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p2name is %s", provider_getName(*p2));
  if(provider_getName(*p1)==NULL && provider_getName(*p2)==NULL)
    return 0;
  if(provider_getName(*p1)==NULL)
    return -1;
  if(provider_getName(*p2)==NULL)
    return 1;
  return strcmp(provider_getName(*p1), provider_getName(*p2));
}

/** @fn void sortProvider()
 * @brief sorts providers by their name using \f provider_comparator 
 */
struct oidc_provider* sortProvider(struct oidc_provider* p, size_t size) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p is %p", p);
  return arr_sort(p, size, sizeof(struct oidc_provider), provider_comparator);
}

struct oidc_provider* findProvider(struct oidc_provider* p, size_t size, struct oidc_provider key) {
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "p is %p", p);
  unsigned int i;
  for (i=0; i<size; i++) {
    char* json = providerToJSON(*(p+i));
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Provider: %s", json);
    free(json);
  }
  return arr_find(p, size, sizeof(struct oidc_provider), &key, provider_comparator);
}

struct oidc_provider* removeProvider(struct oidc_provider* p, size_t* size, struct oidc_provider key) {
    return arr_removeElement(p, size, sizeof(struct oidc_provider), &key, provider_comparator);
}

struct oidc_provider* getProviderFromJSON(char* json) {
  if(NULL==json)
    return NULL;
  struct oidc_provider* p = calloc(sizeof(struct oidc_provider), 1);
  struct key_value pairs[9];
  pairs[0].key = "issuer";
  pairs[1].key = "name";
  pairs[2].key = "client_id";
  pairs[3].key = "client_secret";
  pairs[4].key = "configuration_endpoint";
  pairs[5].key = "token_endpoint";
  pairs[6].key = "username";
  pairs[7].key = "password";
  pairs[8].key = "refresh_token";
  if(getJSONValues(json, pairs, sizeof(pairs)/sizeof(*pairs))>0) {
  provider_setIssuer(p, pairs[0].value);
  provider_setName(p, pairs[1].value);
  provider_setClientId(p, pairs[2].value);
  provider_setClientSecret(p, pairs[3].value);
  provider_setConfigEndpoint(p, pairs[4].value);
  provider_setTokenEndpoint(p, pairs[5].value);
  provider_setUsername(p, pairs[6].value);
  provider_setPassword(p, pairs[7].value);
  provider_setRefreshToken(p, pairs[8].value);
      } 
  if(provider_getIssuer(*p) && provider_getName(*p) && provider_getClientId(*p) && provider_getClientSecret(*p) && provider_getConfigEndpoint(*p) && provider_getTokenEndpoint(*p) && provider_getUsername(*p) && provider_getPassword(*p) && provider_getRefreshToken(*p)) 
        return p;
  freeProvider(p);
  return NULL;
}

char* providerToJSON(struct oidc_provider p) {
  char* fmt = "{\n\"name\":\"%s\",\n\"issuer\":\"%s\",,\n\"configuration_endpoint\":\"%s\",\n\"token_endpoint\":\"%s\",\n\"client_id\":\"%s\",\n\"client_secret\":\"%s\",\n\"username\":\"%s\",\n\"password\":\"%s\",\n\"refresh_token\":\"%s\"\n}";
  char* p_json = calloc(sizeof(char), snprintf(NULL, 0, fmt, p.name, p.issuer, p.configuration_endpoint, p.token_endpoint, p.client_id, p.client_secret, p.username, p.password, p.refresh_token)+1);
  sprintf(p_json, fmt, p.name, p.issuer, p.configuration_endpoint, p.token_endpoint, p.client_id, p.client_secret, p.username, p.password, p.refresh_token);
  return p_json;
}

void freeProvider(struct oidc_provider* p) {
  provider_setName(p, NULL);
  provider_setIssuer(p, NULL);
  provider_setConfigEndpoint(p, NULL);
  provider_setTokenEndpoint(p, NULL);
  provider_setClientId(p, NULL);
  provider_setClientSecret(p, NULL);
  provider_setUsername(p, NULL);
  provider_setPassword(p, NULL);
  provider_setRefreshToken(p, NULL);
  provider_setAccessToken(p, NULL);
  free(p);
}

int providerConfigExists(const char* providername) {
  return oidcFileDoesExist(providername);
}

struct oidc_provider* decryptProvider(const char* providername, const char* password) {
  char* fileText = readOidcFile(providername);
  unsigned long cipher_len = atoi(strtok(fileText, ":"));
  char* salt_hex = strtok(NULL, ":");
  char* nonce_hex = strtok(NULL, ":");
  char* cipher = strtok(NULL, ":");
  unsigned char* decrypted = decrypt(cipher, cipher_len, password, nonce_hex, salt_hex);
  free(fileText);
  struct oidc_provider* p = getProviderFromJSON((char*)decrypted);
  free(decrypted);
  return p;
}


