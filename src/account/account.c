#include "account.h"

#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "issuer_helper.h"
#include "utils/crypt/cryptUtils.h"
#include "utils/crypt/memoryCrypt.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"

#include <syslog.h>

/**
 * @brief compares two accounts by their name.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return @c 1 if the names match, @c 0 otherwise
 */
int account_matchByName(const struct oidc_account* p1,
                        const struct oidc_account* p2) {
  if (account_getName(p1) == NULL && account_getName(p2) == NULL) {
    return 1;
  }
  if (account_getName(p1) == NULL) {
    return 0;
  }
  if (account_getName(p2) == NULL) {
    return 0;
  }
  return strcmp(account_getName(p1), account_getName(p2)) == 0;
}

/**
 * @brief compares two accounts by their name.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return @c 1 if the states match, @c 0 otherwise
 */
int account_matchByState(const struct oidc_account* p1,
                         const struct oidc_account* p2) {
  char* state1 = account_getUsedState(p1);
  char* state2 = account_getUsedState(p2);
  if (state1 == NULL && state2 == NULL) {
    return 1;
  }
  if (state1 == NULL) {
    return 0;
  }
  if (state2 == NULL) {
    return 0;
  }
  return strcmp(state1, state2) == 0;
}

/**
 * reads the pubclient.conf file and updates the account struct if a public
 * client is found for that issuer, also setting the redirect uris
 * @param account the account struct to be updated
 * @return the updated account struct, or @c NULL
 */
struct oidc_account* updateAccountWithPublicClientInfo(
    struct oidc_account* account) {
  if (account == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char*        issuer_url     = account_getIssuerUrl(account);
  list_t*      pubClientLines = getLinesFromFile(ETC_PUBCLIENTS_CONFIG_FILE);
  list_node_t* node;
  list_iterator_t* it = list_iterator_new(pubClientLines, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    char* client = strtok(node->val, "@");
    char* iss    = strtok(NULL, "@");
    // syslog(LOG_AUTHPRIV | LOG_DEBUG, "Found public client for '%s'", iss);
    if (compIssuerUrls(issuer_url, iss)) {
      char* client_id     = strtok(client, ":");
      char* client_secret = strtok(NULL, ":");
      account_setClientId(account, oidc_strcopy(client_id));
      account_setClientSecret(account, oidc_strcopy(client_secret));
      syslog(LOG_AUTHPRIV | LOG_DEBUG,
             "Using public client with id '%s' and secret '%s'", client_id,
             client_secret);
      list_t* redirect_uris =
          createList(0, "http://localhost:8080", "http://localhost:4242",
                     "http://localhost:43985", NULL);
      redirect_uris->match = (int (*)(void*, void*))strequal;
      account_setRedirectUris(account, redirect_uris);
      break;
    }
  }
  list_iterator_destroy(it);
  list_destroy(pubClientLines);
  return account;
}

/**
 * @brief parses a json encoded account
 * @param json the json string
 * @return a pointer a the oidc_account. Has to be freed after usage. On
 * failure NULL is returned.
 */
struct oidc_account* getAccountFromJSON(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct oidc_account* p   = secAlloc(sizeof(struct oidc_account));
  size_t               len = 14;
  struct key_value     pairs[len];
  for (size_t i = 0; i < len; i++) { pairs[i].value = NULL; }
  pairs[0].key  = AGENT_KEY_ISSUERURL;
  pairs[1].key  = OIDC_KEY_ISSUER;
  pairs[2].key  = AGENT_KEY_SHORTNAME;
  pairs[3].key  = OIDC_KEY_CLIENTID;
  pairs[4].key  = OIDC_KEY_CLIENTSECRET;
  pairs[5].key  = OIDC_KEY_USERNAME;
  pairs[6].key  = OIDC_KEY_PASSWORD;
  pairs[7].key  = OIDC_KEY_REFRESHTOKEN;
  pairs[8].key  = AGENT_KEY_CERTPATH;
  pairs[9].key  = OIDC_KEY_REDIRECTURIS;
  pairs[10].key = OIDC_KEY_SCOPE;
  pairs[11].key = OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT;
  pairs[12].key = OIDC_KEY_CLIENTNAME;
  pairs[13].key = AGENT_KEY_DAESETBYUSER;
  if (getJSONValuesFromString(json, pairs, sizeof(pairs) / sizeof(*pairs)) >
      0) {
    struct oidc_issuer* iss = secAlloc(sizeof(struct oidc_issuer));
    if (pairs[0].value) {
      issuer_setIssuerUrl(iss, pairs[0].value);
      secFree(pairs[1].value);
    } else {
      issuer_setIssuerUrl(iss, pairs[1].value);
    }
    issuer_setDeviceAuthorizationEndpoint(iss, pairs[11].value,
                                          strToInt(pairs[13].value));
    secFree(pairs[13].value);
    account_setIssuer(p, iss);
    account_setName(p, pairs[2].value, NULL);
    account_setClientName(p, pairs[12].value);
    account_setClientId(p, pairs[3].value);
    account_setClientSecret(p, pairs[4].value);
    account_setUsername(p, pairs[5].value);
    account_setPassword(p, pairs[6].value);
    account_setRefreshToken(p, pairs[7].value);
    account_setCertPath(p, pairs[8].value);
    account_setScope(p, pairs[10].value);
    list_t* redirect_uris = JSONArrayStringToList(pairs[9].value);
    account_setRedirectUris(p, redirect_uris);
    secFree(pairs[9].value);
    return p;
  }
  secFreeAccount(p);
  return NULL;
}

char* accountToJSONString(const struct oidc_account* p) {
  cJSON* json = accountToJSON(p);
  char*  str  = jsonToString(json);
  secFreeJson(json);
  return str;
}

char* accountToJSONStringWithoutCredentials(const struct oidc_account* p) {
  cJSON* json = accountToJSONWithoutCredentials(p);
  char*  str  = jsonToString(json);
  secFreeJson(json);
  return str;
}

cJSON* _accountToJSON(const struct oidc_account* p, int useCredentials) {
  cJSON* redirect_uris = listToJSONArray(account_getRedirectUris(p));
  char*  refresh_token = account_getRefreshToken(p);
  cJSON* json          = generateJSONObject(
      AGENT_KEY_SHORTNAME, cJSON_String,
      strValid(account_getName(p)) ? account_getName(p) : "",
      OIDC_KEY_CLIENTNAME, cJSON_String,
      strValid(account_getClientName(p)) ? account_getClientName(p) : "",
      AGENT_KEY_ISSUERURL, cJSON_String,
      strValid(account_getIssuerUrl(p)) ? account_getIssuerUrl(p) : "",
      OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT, cJSON_String,
      strValid(account_getDeviceAuthorizationEndpoint(p))
          ? account_getDeviceAuthorizationEndpoint(p)
          : "",
      AGENT_KEY_DAESETBYUSER, cJSON_Number,
      account_getIssuer(p) ? issuer_getDeviceAuthorizationEndpointIsSetByUser(
                                 account_getIssuer(p))
                           : 0,
      OIDC_KEY_CLIENTID, cJSON_String,
      strValid(account_getClientId(p)) ? account_getClientId(p) : "",
      OIDC_KEY_CLIENTSECRET, cJSON_String,
      strValid(account_getClientSecret(p)) ? account_getClientSecret(p) : "",
      OIDC_KEY_REFRESHTOKEN, cJSON_String,
      strValid(refresh_token) ? refresh_token : "", AGENT_KEY_CERTPATH,
      cJSON_String,
      strValid(account_getCertPath(p)) ? account_getCertPath(p) : "",
      OIDC_KEY_SCOPE, cJSON_String,
      strValid(account_getScope(p)) ? account_getScope(p) : "", NULL);
  jsonAddJSON(json, OIDC_KEY_REDIRECTURIS, redirect_uris);
  if (useCredentials) {
    jsonAddStringValue(
        json, OIDC_KEY_USERNAME,
        strValid(account_getUsername(p)) ? account_getUsername(p) : "");
    jsonAddStringValue(
        json, OIDC_KEY_PASSWORD,
        strValid(account_getPassword(p)) ? account_getPassword(p) : "");
  }
  return json;
}

/**
 * @brief converts an account into a json string
 * @param p a pointer to the oidc_account to be converted
 * @return a poitner to a json string representing the account. Has to be freed
 * after usage.
 */
cJSON* accountToJSON(const struct oidc_account* p) {
  return _accountToJSON(p, 1);
}

cJSON* accountToJSONWithoutCredentials(const struct oidc_account* a) {
  return _accountToJSON(a, 0);
}

/** void freeAccount(struct oidc_account* p)
 * @brief frees an account completly including all fields.
 * @param p a pointer to the account to be freed
 */
void _secFreeAccount(struct oidc_account* p) {
  if (p == NULL) {
    return;
  }
  secFreeAccountContent(p);
  secFree(p);
}

/** void freeAccountContent(struct oidc_account* p)
 * @brief frees a all fields of an account. Does not free the pointer it self
 * @param p a pointer to the account to be freed
 */
void secFreeAccountContent(struct oidc_account* p) {
  if (p == NULL) {
    return;
  }
  account_setName(p, NULL, NULL);
  // account_setClientName(p, NULL); //Included in account_setName
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

/**
 * @brief  decrypts the passed configuration file content into an oidc_account
 * @param fileText the content of the oidc account configuration file
 * @param password the encryption password
 * @return a pointer to an oidc_account. Has to be freed after usage. Null on
 * failure.
 */
struct oidc_account* decryptAccountText(const char* fileText,
                                        const char* password) {
  if (fileText == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* decrypted = decryptFileContent(fileText, password);
  if (NULL == decrypted) {
    return NULL;
  }
  struct oidc_account* p = getAccountFromJSON((char*)decrypted);
  secFree(decrypted);
  return p;
}

/**
 * @brief reads the encrypted configuration for a given short name and decrypts
 * the configuration.
 * @param accountname the short name of the account that should be decrypted
 * @param password the encryption password
 * @return a pointer to an oidc_account. Has to be freed after usage. Null on
 * failure.
 */
struct oidc_account* decryptAccount(const char* accountname,
                                    const char* password) {
  if (accountname == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  char* decrypted = decryptOidcFile(accountname, password);
  if (NULL == decrypted) {
    return NULL;
  }
  struct oidc_account* p = getAccountFromJSON((char*)decrypted);
  secFree(decrypted);
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
  list_t*          stringList = list_new();
  list_node_t*     node;
  list_iterator_t* it = list_iterator_new(accounts, LIST_HEAD);
  while ((node = list_iterator_next(it))) {
    list_rpush(stringList,
               list_node_new(account_getName((struct oidc_account*)node->val)));
  }
  list_iterator_destroy(it);
  char* str = listToJSONArrayString(stringList);
  list_destroy(stringList);
  return str;
}

int hasRedirectUris(const struct oidc_account* account) {
  char* str = listToDelimitedString(account_getRedirectUris(account), ' ');
  int   ret = str != NULL ? 1 : 0;
  secFree(str);
  return ret;
}

list_t* defineUsableScopeList(const struct oidc_account* account) {
  list_t* supported =
      delimitedStringToList(account_getScopesSupported(account), ' ');
  if (supported != NULL) {
    list_addStringIfNotFound(supported, OIDC_SCOPE_OPENID);
    if (!compIssuerUrls(account_getIssuerUrl(account), GOOGLE_ISSUER_URL)) {
      list_addStringIfNotFound(supported, OIDC_SCOPE_OFFLINE_ACCESS);
    }
  }
  // extern void _printList(list_t * l);
  // printf("supported\n");
  // _printList(supported);
  char* wanted_str = account_getScope(account);
  if (wanted_str && strequal(wanted_str, AGENT_SCOPE_ALL)) {
    if (supported == NULL) {
      return delimitedStringToList(wanted_str, ' ');
    }
    return supported;
  }
  list_t* wanted = delimitedStringToList(wanted_str, ' ');
  if (wanted == NULL) {
    wanted = createList(1, NULL);
  }
  list_addStringIfNotFound(wanted, OIDC_SCOPE_OPENID);
  list_addStringIfNotFound(wanted, OIDC_SCOPE_OFFLINE_ACCESS);

  // printf("wanted\n");
  // _printList(wanted);
  if (supported == NULL) {
    if (compIssuerUrls(account_getIssuerUrl(account), GOOGLE_ISSUER_URL)) {
      list_removeIfFound(wanted, OIDC_SCOPE_OFFLINE_ACCESS);
    }
    return wanted;
  }
  list_t* scopes = intersectLists(wanted, supported);
  list_destroy(wanted);
  list_destroy(supported);
  return scopes;
}

char* defineUsableScopes(const struct oidc_account* account) {
  list_t* scopes = defineUsableScopeList(account);
  if (scopes == NULL) {
    return NULL;
  }
  char* usable = listToDelimitedString(scopes, ' ');
  list_destroy(scopes);
  syslog(LOG_AUTHPRIV | LOG_DEBUG, "usable scope is '%s'", usable);
  return usable;
}

void account_setRefreshToken(struct oidc_account* p, char* refresh_token) {
  if (p->refresh_token == refresh_token) {
    return;
  }
  secFree(p->refresh_token);
  p->refresh_token = refresh_token;
}

char* account_getRefreshToken(const struct oidc_account* p) {
  return p ? p->refresh_token : NULL;
}

int account_refreshTokenIsValid(const struct oidc_account* p) {
  char* refresh_token = account_getRefreshToken(p);
  int   ret           = strValid(refresh_token);
  return ret;
}
