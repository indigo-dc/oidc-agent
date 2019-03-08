#include "account.h"
#include "defines/agent_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "issuer_helper.h"
#include "utils/file_io/cryptFileUtils.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/file_io/promptCryptFileUtils.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/matcher.h"

#include <syslog.h>

/**
 * @brief compares two accounts by their name.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return @c 1 if the names match, @c 0 otherwise
 */
int account_matchByName(const struct oidc_account* p1,
                        const struct oidc_account* p2) {
  return matchStrings(account_getName(p1), account_getName(p2));
}

/**
 * @brief compares two accounts by their name.
 * @param v1 pointer to the first element
 * @param v2 pointer to the second element
 * @return @c 1 if the states match, @c 0 otherwise
 */
int account_matchByState(const struct oidc_account* p1,
                         const struct oidc_account* p2) {
  return matchStrings(account_getUsedState(p1), account_getUsedState(p2));
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
      redirect_uris->match = (matchFunction)strequal;
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
  INIT_KEY_VALUE(AGENT_KEY_ISSUERURL, OIDC_KEY_ISSUER, AGENT_KEY_SHORTNAME,
                 OIDC_KEY_CLIENTID, OIDC_KEY_CLIENTSECRET, OIDC_KEY_USERNAME,
                 OIDC_KEY_PASSWORD, OIDC_KEY_REFRESHTOKEN, AGENT_KEY_CERTPATH,
                 OIDC_KEY_REDIRECTURIS, OIDC_KEY_SCOPE,
                 OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT, OIDC_KEY_CLIENTNAME,
                 AGENT_KEY_DAESETBYUSER);
  GET_JSON_VALUES_RETURN_NULL_ONERROR(json);
  KEY_VALUE_VARS(issuer_url, issuer, shortname, client_id, client_secret,
                 username, password, refresh_token, cert_path, redirect_uris,
                 scope, device_authorization_endpoint, clientname,
                 daeSetByUser);
  struct oidc_account* p   = secAlloc(sizeof(struct oidc_account));
  struct oidc_issuer*  iss = secAlloc(sizeof(struct oidc_issuer));
  if (_issuer_url) {
    issuer_setIssuerUrl(iss, _issuer_url);
    secFree(_issuer);
  } else {
    issuer_setIssuerUrl(iss, _issuer);
  }
  issuer_setDeviceAuthorizationEndpoint(iss, _device_authorization_endpoint,
                                        strToInt(_daeSetByUser));
  secFree(_daeSetByUser);
  account_setIssuer(p, iss);
  account_setName(p, _shortname, NULL);
  account_setClientName(p, _clientname);
  account_setClientId(p, _client_id);
  account_setClientSecret(p, _client_secret);
  account_setUsername(p, _username);
  account_setPassword(p, _password);
  account_setRefreshToken(p, _refresh_token);
  account_setCertPath(p, _cert_path);
  account_setScopeExact(p, _scope);
  list_t* redirect_uris = JSONArrayStringToList(_redirect_uris);
  account_setRedirectUris(p, redirect_uris);
  secFree(_redirect_uris);
  return p;
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
  account_setScopeExact(p, NULL);
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
 * @brief reads the encrypted configuration for a given short name and decrypts
 * the configuration.
 * @param accountname the short name of the account that should be decrypted
 * @param password the encryption password
 * @return a pointer to an oidc_account. Has to be freed after usage. Null on
 * failure.
 */
struct oidc_account* getDecryptedAccountFromFile(const char* accountname,
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

/**
 * @brief reads the encrypted configuration for a given short name and decrypts
 * the configuration.
 * @param accountname the short name of the account that should be decrypted
 * @param pw_cmd  the command used to get the encryption password, can be
 * @c NULL
 * @return a pointer to an oidc_account. Has to be freed after usage. Null on
 * failure.
 */
struct resultWithEncryptionPassword
getDecryptedAccountAndPasswordFromFilePrompt(const char* accountname,
                                             const char* pw_cmd) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return RESULT_WITH_PASSWORD_NULL;
  }
  struct resultWithEncryptionPassword result =
      getDecryptedOidcFileAndPasswordFor(accountname, pw_cmd);
  char* config = result.result;
  if (NULL == config) {
    return result;
  }
  struct oidc_account* p = getAccountFromJSON(config);
  secFree(config);
  result.result = p;
  return result;
}

struct oidc_account* getDecryptedAccountFromFilePrompt(const char* accountname,
                                                       const char* pw_cmd) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct resultWithEncryptionPassword result =
      getDecryptedOidcFileAndPasswordFor(accountname, pw_cmd);
  secFree(result.password);
  char* config = result.result;
  if (NULL == config) {
    return NULL;
  }
  struct oidc_account* p = getAccountFromJSON(config);
  secFree(config);
  return p;
}

struct resultWithEncryptionPassword
getDecryptedAccountAsStringAndPasswordFromFilePrompt(const char* accountname,
                                                     const char* pw_cmd) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return RESULT_WITH_PASSWORD_NULL;
  }
  struct resultWithEncryptionPassword result =
      getDecryptedAccountAndPasswordFromFilePrompt(accountname, pw_cmd);
  if (NULL == result.result) {
    return result;
  }
  char* json = accountToJSONString(result.result);
  secFreeAccount(result.result);
  result.result = json;
  return result;
}

char* getDecryptedAccountAsStringFromFilePrompt(const char* accountname,
                                                const char* pw_cmd) {
  if (accountname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  struct resultWithEncryptionPassword result =
      getDecryptedAccountAsStringAndPasswordFromFilePrompt(accountname, pw_cmd);
  secFree(result.password);
  return result.result;
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
  // printf("wanted str: %s\n", wanted_str);
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

void stringifyIssuerUrl(struct oidc_account* account) {
  account_setIssuerUrl(account,
                       withTrailingSlash(account_getIssuerUrl(account)));
}

void account_setOSDefaultCertPath(struct oidc_account* account) {
  for (unsigned int i = 0;
       i < sizeof(possibleCertFiles) / sizeof(*possibleCertFiles); i++) {
    if (fileDoesExist(possibleCertFiles[i])) {
      account_setCertPath(account, oidc_strcopy(possibleCertFiles[i]));
      return;
    }
  }
}
