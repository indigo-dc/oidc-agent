#include "account.h"

#include "json.h"
#include "crypt.h"
#include "file_io/oidc_file_io.h"
#include "utils/fileUtils.h"
#include "utils/listUtils.h"

#include <syslog.h>


/** @fn int account_comparator(const void* v1, const void* v2)
 * @brief compares two accounts by their name. Can be used for sorting.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
int account_matchByName(struct oidc_account* p1, struct oidc_account* p2) {
  if(account_getName(*p1)==NULL && account_getName(*p2)==NULL) {
    return 1;
  }
  if(account_getName(*p1)==NULL || account_getName(*p2)==NULL) {
    return 0;
  }
  return strcmp(account_getName(*p1), account_getName(*p2)) == 0;
}

/** @fn int account_comparator(const void* v1, const void* v2)
 * @brief compares two accounts by their name. Can be used for sorting.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return -1 if v1<v2; 1 if v1>v2; 0 if v1=v2
 */
int account_matchByState(struct oidc_account* p1, struct oidc_account* p2) {
  if(account_getUsedState(*p1)==NULL && account_getUsedState(*p2)==NULL) {
    return 0;
  }
  if(account_getUsedState(*p1)==NULL || account_getUsedState(*p2)==NULL) {
    return 0;
  }
  return strcmp(account_getUsedState(*p1), account_getUsedState(*p2)) == 0;
}

/** @fn struct oidc_account* getAccountFromJSON(char* json)
 * @brief parses a json encoded account
 * @param json the json string
 * @return a pointer a the oidc_account. Has to be freed after usage. On
 * failure NULL is returned.
 */
struct oidc_account* getAccountFromJSON(char* json) {
  if(NULL==json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct oidc_account* p = calloc(sizeof(struct oidc_account), 1);
  struct key_value pairs[11];
  pairs[0].key = "issuer_url"; pairs[0].value = NULL;
  pairs[1].key = "issuer"; pairs[1].value = NULL;
  pairs[2].key = "name"; pairs[2].value = NULL;
  pairs[3].key = "client_id"; pairs[3].value = NULL;
  pairs[4].key = "client_secret"; pairs[4].value = NULL;
  pairs[5].key = "username"; pairs[5].value = NULL;
  pairs[6].key = "password"; pairs[6].value = NULL;
  pairs[7].key = "refresh_token"; pairs[7].value = NULL; pairs[8].key = "cert_path"; pairs[8].value = NULL;
  pairs[9].key = "redirect_uris"; pairs[9].value = NULL;
  pairs[10].key = "scope"; pairs[10].value = NULL;
  if(getJSONValues(json, pairs, sizeof(pairs)/sizeof(*pairs))>0) {
    struct oidc_issuer* iss = calloc(sizeof(struct oidc_issuer), 1);
    if(pairs[0].value) {
      issuer_setIssuerUrl(iss, pairs[0].value);
      clearFreeString(pairs[1].value);
    } else {
      issuer_setIssuerUrl(iss, pairs[1].value);
    }
    account_setIssuer(p, iss);
    account_setName(p, pairs[2].value);
    account_setClientId(p, pairs[3].value);
    account_setClientSecret(p, pairs[4].value);
    account_setUsername(p, pairs[5].value);
    account_setPassword(p, pairs[6].value);
    account_setRefreshToken(p, pairs[7].value);
    account_setCertPath(p, pairs[8].value);
    account_setScope(p, pairs[10].value);
    list_t* redirect_uris = JSONArrayToList(pairs[9].value); 
    account_setRedirectUris(p, redirect_uris);
    clearFreeString(pairs[9].value);
    return p;
  } 
  clearFreeAccount(p);
  return NULL;
}

char* _accountToJSON(struct oidc_account p, int useCredentials) {
char* redirect_uris = calloc(sizeof(char), 2+1);
  strcpy(redirect_uris, "[]");;
  unsigned int i;
  for(i=0; i<account_getRedirectUrisCount(p); i++) {
    redirect_uris = json_arrAdd(redirect_uris, list_at(account_getRedirectUris(p), i)->val);
  }
  char* json = useCredentials ?
    generateJSONObject(
      "name", strValid(account_getName(p)) ? account_getName(p) : "", 1,
      "issuer_url", strValid(account_getIssuerUrl(p)) ? account_getIssuerUrl(p) : "", 1,
      "client_id", strValid(account_getClientId(p)) ? account_getClientId(p) : "", 1,
      "client_secret", strValid(account_getClientSecret(p)) ? account_getClientSecret(p) : "", 1,
      "refresh_token", strValid(account_getRefreshToken(p)) ? account_getRefreshToken(p) : "", 1,
      "cert_path", strValid(account_getCertPath(p)) ? account_getCertPath(p) : "", 1,
      "username", strValid(account_getUsername(p)) ? account_getUsername(p) : "", 1,
      "password", strValid(account_getPassword(p)) ? account_getPassword(p) : "", 1,
      "redirect_uris", redirect_uris, 0,
      "scope", strValid(account_getScope(p)) ? account_getScope(p) : "", 1,
      NULL
      ) : 
    generateJSONObject(
      "name", strValid(account_getName(p)) ? account_getName(p) : "", 1,
      "issuer_url", strValid(account_getIssuerUrl(p)) ? account_getIssuerUrl(p) : "", 1,
      "client_id", strValid(account_getClientId(p)) ? account_getClientId(p) : "", 1,
      "client_secret", strValid(account_getClientSecret(p)) ? account_getClientSecret(p) : "", 1,
      "refresh_token", strValid(account_getRefreshToken(p)) ? account_getRefreshToken(p) : "", 1,
      "cert_path", strValid(account_getCertPath(p)) ? account_getCertPath(p) : "", 1,
      "redirect_uris", redirect_uris, 0,
      "scope", strValid(account_getScope(p)) ? account_getScope(p) : "", 1,
      NULL
      );
  clearFreeString(redirect_uris);
  return json;

}

/** @fn char* accountToJSON(struct oidc_rovider p)
 * @brief converts a account into a json string
 * @param p the oidc_account to be converted
 * @return a poitner to a json string representing the account. Has to be freed
 * after usage.
 */
char* accountToJSON(struct oidc_account a) {
  return _accountToJSON(a, 1);
  }

char* accountToJSONWithoutCredentials(struct oidc_account a) {
  return _accountToJSON(a, 0);
}

/** void freeAccount(struct oidc_account* p)
 * @brief frees a account completly including all fields.
 * @param p a pointer to the account to be freed
 */
void clearFreeAccount(struct oidc_account* p) {
  if(p==NULL) {
    return;
  }
  clearFreeAccountContent(p);
  clearFree(p, sizeof(*p));
}

/** void freeAccountContent(struct oidc_account* p)
 * @brief frees a all fields of a account. Does not free the pointer it self
 * @param p a pointer to the account to be freed
 */
void clearFreeAccountContent(struct oidc_account* p) {
  if(p==NULL) {
    return;
  }
  account_setName(p, NULL);
  account_setIssuer(p, NULL);
  account_setClientId(p, NULL);
  account_setClientSecret(p, NULL);
  account_setScope(p, NULL);
  account_setUsername(p, NULL);
  account_setPassword(p, NULL);
  account_setRefreshToken(p, NULL);
  account_setAccessToken(p, NULL);
  account_setCertPath(p, NULL);
  account_setRedirectUris(p, NULL);
  account_setUsedState(p, NULL);
}

/** int accountconfigExists(const char* accountname)
 * @brief checks if a configuration for a given account exists
 * @param accountname the short name that should be checked
 * @return 1 if the configuration exists, 0 if not
 */
int accountConfigExists(const char* accountname) {
  return oidcFileDoesExist(accountname);
}

/** @fn struct oidc_account* decryptAccount(const char* accountname, const char* password) 
 * @brief reads the encrypted configuration for a given short name and decrypts
 * the configuration.
 * @param accountname the short name of the account that should be decrypted
 * @param password the encryption password
 * @return a pointer to an oidc_account. Has to be freed after usage. Null on
 * failure.
 */
struct oidc_account* decryptAccount(const char* accountname, const char* password) {
  char* fileText = readOidcFile(accountname);
  struct oidc_account* p = decryptAccountText(fileText, password);
  clearFreeString(fileText);
  return p;
}

struct oidc_account* decryptAccountText(char* fileContent, const char* password) {
  if(fileContent==NULL || password ==NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  unsigned char* decrypted = decryptFileContent(fileContent, password); 
  if(NULL==decrypted) {
    return NULL;
  }
  struct oidc_account* p = getAccountFromJSON((char*)decrypted);
  clearFreeString((char*)decrypted);
  return p;
}

/** @fn char* getAccountNameList(struct oidc_account* p, size_t size) 
 * @brief gets the account short names from an array of accounts
 * @param p a pointer to the first account
 * @param size the nubmer of accounts
 * @return a pointer to a JSON Array String containing all the account names.
 * Has to be freed after usage.
 */
char* getAccountNameList(list_t* accounts) {
  list_t* stringList = list_new();
  list_node_t* node;
  list_iterator_t* it = list_iterator_new(accounts, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    list_rpush(stringList, list_node_new(account_getName(*(struct oidc_account*)node->val)));
  }
  list_iterator_destroy(it);
  char* str = listToJSONArray(stringList);  
  list_destroy(stringList);
  return str;
}

int hasRedirectUris(struct oidc_account account) {
  char* str = listToDelimitedString(account_getRedirectUris(account), ' ');
  int ret = str != NULL ? 1 : 0;
  clearFreeString(str);
  return ret;
}

char* defineUsableScopes(struct oidc_account account) {
  char* supported = oidc_strcopy(account_getScopesSupported(account));
  char* wanted = account_getScope(account);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "supported scope is '%s'", supported);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "wanted scope is '%s'", wanted);
    if(!strValid(supported)) {
      clearFreeString(supported);
    return oidc_strcopy(wanted); //Do not return wanted directly, because it will be used in a call to setScope which will free and then set it
  }
  if(!strValid(wanted)) {
      clearFreeString(supported);
    return NULL;
  }
  
  //Adding mandatory scopes (for oidc-agent) to supported scopes
  if(strstr(supported, "openid")==NULL) {
    char* tmp = oidc_strcat(supported, " openid");
    clearFreeString(supported);
    supported = tmp;
  }
  if(strstr(supported, "offline_access")==NULL &&strcmp(account_getIssuerUrl(account), "https://accounts.google.com/")!=0) { //don't add offline_access for google, because theay don't accept it
    char* tmp = oidc_strcat(supported, " offline_access");
    clearFreeString(supported);
    supported = tmp;
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "supported scope is now '%s'", supported);

  if(strcmp("max", wanted)==0) {
    return supported;
  }
  list_t* list = delimitedStringToList(wanted, ' ');
  list_node_t *node;
  list_iterator_t *it = list_iterator_new(list, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    if(strstr(supported, node->val)==NULL) {
      list_remove(list, node);
    }
  }
  clearFreeString(supported);
  char* usable = listToDelimitedString(list, ' ');

  list_iterator_destroy(it);
  list_destroy(list);
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "usable scope is '%s'", usable);
  return usable;
}
