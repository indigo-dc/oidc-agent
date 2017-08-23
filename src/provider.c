#include <stdio.h>
#include <string.h>

#include <syslog.h>

#include "provider.h"
#include "file_io.h"
#include "crypt.h"
#include "oidc_array.h"
#include "oidc_utilities.h"
#include "oidc_error.h"

/** @fn struct oidc_provider* addProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider)   
 * @brief adds a provider to an array 
 * @param p a pointer to the start of an array
 * @param size a pointer to the number of providers in the array
 * @param provider the provider to be added. 
 * @return a pointer to the new array
 */
struct oidc_provider* addProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider) {
  void* tmp = realloc(p, sizeof(struct oidc_provider) * (*size + 1));
  if(tmp==NULL) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  p = tmp;
  memcpy(p + *size, &provider, sizeof(struct oidc_provider));
  (*size)++;
  // For some reason using the following function insted of the above same
  // statements doesn't work.
  // p= arr_addElement(p, size, sizeof(struct oidc_provider), &provider);    
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
  if(provider_getName(*p1)==NULL && provider_getName(*p2)==NULL) {
    return 0;
  }
  if(provider_getName(*p1)==NULL) {
    return -1;
  }
  if(provider_getName(*p2)==NULL) {
    return 1;
  }
  return strcmp(provider_getName(*p1), provider_getName(*p2));
}

/** @fn void sortProvider()
 * @brief sorts providers by their name using \f provider_comparator 
 * @param p the array to be sorted
 * @param size the number of providers in \p p
 * @return the sorted array
 */
struct oidc_provider* sortProvider(struct oidc_provider* p, size_t size) {
  return arr_sort(p, size, sizeof(struct oidc_provider), provider_comparator);
}

/** @fn struct oidc_provider* findProvider(struct oidc_provider* p, size_t size, struct oidc_provider key) 
 * @brief finds a provider in an array.
 * @param p the array that should be searched
 * @param size the number of elements in arr
 * @param key the provider to be found. 
 * @return a pointer to the found provider. If no provider could be found
 * NULL is returned.
 */
struct oidc_provider* findProvider(struct oidc_provider* p, size_t size, struct oidc_provider key) {
  return arr_find(p, size, sizeof(struct oidc_provider), &key, provider_comparator);
}

/** @fn struct oidc_provider* removeProvider(struct oidc_provider* p, size_t* size, struct oidc_provider provider)   
 * @brief removes a provider from an array 
 * @param p a pointer to the start of an array
 * @param size a pointer to the number of providers in the array
 * @param provider the provider to be removed. 
 * @return a pointer to the new array
 */
struct oidc_provider* removeProvider(struct oidc_provider* p, size_t* size, struct oidc_provider key) {
  void* pos = findProvider(p, *size,  key);
  if(NULL==pos) {
    return p;
  }
  freeProviderContent(pos);
  memmove(pos, p + *size - 1, sizeof(struct oidc_provider));
  (*size)--;
  void* tmp = realloc(p, sizeof(struct oidc_provider) * (*size));
  if(tmp==NULL && *size > 0) {
    syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
    oidc_errno = OIDC_EALLOC;
    return NULL;
  }
  p = tmp;
  return p;

}

/** @fn struct oidc_provider* getProviderFromJSON(char* json)
 * @brief parses a json encoded provider
 * @param json the json string
 * @return a pointer a the oidc_provider. Has to be freed after usage. On
 * failure NULL is returned.
 */
struct oidc_provider* getProviderFromJSON(char* json) {
  if(NULL==json) {
    return NULL;
  }
  struct oidc_provider* p = calloc(sizeof(struct oidc_provider), 1);
  struct key_value pairs[10];
  pairs[0].key = "issuer";
  pairs[1].key = "name";
  pairs[2].key = "client_id";
  pairs[3].key = "client_secret";
  pairs[4].key = "configuration_endpoint";
  pairs[5].key = "token_endpoint";
  pairs[6].key = "username";
  pairs[7].key = "password";
  pairs[8].key = "refresh_token";
  pairs[9].key = "cert_path";
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
    provider_setCertPath(p, pairs[9].value);
  } 
  if(provider_getIssuer(*p) && provider_getName(*p) && provider_getClientId(*p) && provider_getClientSecret(*p) && provider_getConfigEndpoint(*p) && provider_getTokenEndpoint(*p) && provider_getUsername(*p) && provider_getPassword(*p) && provider_getRefreshToken(*p) && provider_getCertPath(*p)) {
    return p;
  }
  freeProvider(p);
  return NULL;
}

/** @fn char* providerToJSON(struct oidc_rovider p)
 * @brief converts a provider into a json string
 * @param p the oidc_provider to be converted
 * @return a poitner to a json string representing the provider. Has to be freed
 * after usage.
 */
char* providerToJSON(struct oidc_provider p) {
  char* fmt = "{\n\"name\":\"%s\",\n\"issuer\":\"%s\",,\n\"configuration_endpoint\":\"%s\",\n\"token_endpoint\":\"%s\",\n\"client_id\":\"%s\",\n\"client_secret\":\"%s\",\n\"username\":\"%s\",\n\"password\":\"%s\",\n\"refresh_token\":\"%s\",\n\"cert_path\":\"%s\"\n}";
  char* p_json = calloc(sizeof(char), snprintf(NULL, 0, fmt, 
        isValid(provider_getName(p)) ? provider_getName(p) : "", 
        isValid(provider_getIssuer(p)) ? provider_getIssuer(p) : "", 
        isValid(provider_getConfigEndpoint(p)) ? provider_getConfigEndpoint(p) : "", 
        isValid(provider_getTokenEndpoint(p)) ? provider_getTokenEndpoint(p) : "", 
        isValid(provider_getClientId(p)) ? provider_getClientId(p) : "", 
        isValid(provider_getClientSecret(p)) ? provider_getClientSecret(p) : "", 
        isValid(provider_getUsername(p)) ? provider_getUsername(p) : "", 
        isValid(provider_getPassword(p)) ? provider_getPassword(p) : "", 
        isValid(provider_getRefreshToken(p)) ? provider_getRefreshToken(p) : "", 
        isValid(provider_getCertPath(p)) ? provider_getCertPath(p) : "" 
        )+1);
  sprintf(p_json, fmt, 
      isValid(provider_getName(p)) ? provider_getName(p) : "", 
        isValid(provider_getIssuer(p)) ? provider_getIssuer(p) : "", 
        isValid(provider_getConfigEndpoint(p)) ? provider_getConfigEndpoint(p) : "", 
        isValid(provider_getTokenEndpoint(p)) ? provider_getTokenEndpoint(p) : "", 
        isValid(provider_getClientId(p)) ? provider_getClientId(p) : "", 
        isValid(provider_getClientSecret(p)) ? provider_getClientSecret(p) : "", 
        isValid(provider_getUsername(p)) ? provider_getUsername(p) : "", 
        isValid(provider_getPassword(p)) ? provider_getPassword(p) : "", 
        isValid(provider_getRefreshToken(p)) ? provider_getRefreshToken(p) : "", 
        isValid(provider_getCertPath(p)) ? provider_getCertPath(p) : "" 
      );
  return p_json;
}

/** void freeProvider(struct oidc_provider* p)
 * @brief frees a provider completly including all fields.
 * @param p a pointer to the provider to be freed
 */
void freeProvider(struct oidc_provider* p) {
  freeProviderContent(p);
  clearFree(p, sizeof(*p));
}

/** void freeProviderContent(struct oidc_provider* p)
 * @brief frees a all fields of a provider. Does not free the pointer it self
 * @param p a pointer to the provider to be freed
 */
void freeProviderContent(struct oidc_provider* p) {
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
  provider_setCertPath(p, NULL);
}

/** int providerconfigExists(const char* providername)
 * @brief checks if a configuration for a given provider exists
 * @param providername the short name that should be checked
 * @return 1 if the configuration exists, 0 if not
 */
int providerConfigExists(const char* providername) {
  return oidcFileDoesExist(providername);
}

/** @fn struct oidc_provider* decryptProvider(const char* providername, const char* password) 
 * @brief reads the encrypted configuration for a given short name and decrypts
 * the configuration.
 * @param providername the short name of the provider that should be decrypted
 * @param password the encryption password
 * @return a pointer to an oidc_provider. Has to be freed after usage. Null on
 * failure.
 */
struct oidc_provider* decryptProvider(const char* providername, const char* password) {
  char* fileText = readOidcFile(providername);
  unsigned long cipher_len = atoi(strtok(fileText, ":"));
  char* salt_hex = strtok(NULL, ":");
  char* nonce_hex = strtok(NULL, ":");
  char* cipher = strtok(NULL, ":");
  unsigned char* decrypted = decrypt(cipher, cipher_len, password, nonce_hex, salt_hex);
  clearFreeString(fileText);
  if(NULL==decrypted) {
    return NULL;
  }
  struct oidc_provider* p = getProviderFromJSON((char*)decrypted);
  clearFreeString((char*)decrypted);
  return p;
}

/** @fn char* getProviderNameList(struct oidc_provider* p, size_t size) 
 * @brief gets the provider short names from an array of providers
 * @param p a pointer to the first provider
 * @param size the nubmer of providers
 * @return a poitner to a string contains all the short names, comma separated.
 * Has to be freed after usage.
 */
char* getProviderNameList(struct oidc_provider* p, size_t size) {
  if(NULL==p) {
    oidc_errno = OIDC_EARGNULL;
    return NULL;
  }
  if(0==size) {
    return calloc(sizeof(char), 1);
  }
  unsigned int i;
  char* providerList = calloc(sizeof(char), strlen(provider_getName(*p))+1);
  sprintf(providerList, "%s", provider_getName(*p));
  char* fmt = "%s, %s";
  for(i=1; i<size; i++) {
    char* tmp = realloc(providerList, strlen(providerList)+strlen(provider_getName(*(p+i)))+strlen(fmt)+1);
    if(tmp==NULL) {
      syslog(LOG_AUTHPRIV|LOG_EMERG, "%s (%s:%d) realloc() failed: %m\n", __func__, __FILE__, __LINE__);
      clearFreeString(providerList);
      oidc_errno = OIDC_EALLOC;
      return NULL;
    }
    providerList = tmp;
    sprintf(providerList, fmt, providerList, provider_getName(*(p+i)));
  }
  return providerList;
}

