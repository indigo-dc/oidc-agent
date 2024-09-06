#include "issuerConfig.h"

#include <stdlib.h>
#include <string.h>

#include "defines/agent_values.h"
#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "defines/settings.h"
#include "utils/file_io/fileUtils.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/matcher.h"
#include "utils/memory.h"
#include "utils/oidc_error.h"
#include "utils/string/stringUtils.h"

void _secFreePubclientConfig(struct clientConfig* p) {
  if (p == NULL) {
    return;
  }
  secFree(p->client_id);
  secFree(p->client_secret);
  secFree(p->scope);
  secFreeList(p->flows);
  secFree(p);
}

void _secFreeIssuerConfig(struct issuerConfig* c) {
  if (c == NULL) {
    return;
  }
  secFree(c->issuer);
  secFree(c->contact);
  secFree(c->manual_register);
  secFree(c->configuration_endpoint);
  secFree(c->device_authorization_endpoint);
  secFree(c->cert_path);
  secFreePubclientConfig(c->pub_client);
  secFree(c->default_account);
  secFreeList(c->accounts);
  secFree(c);
}

struct clientConfig* getClientConfigFromJSON(const char* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  INIT_KEY_VALUE(OIDC_KEY_CLIENTID, OIDC_KEY_CLIENTSECRET, OIDC_KEY_SCOPE,
                 IPC_KEY_FLOW, OIDC_KEY_REDIRECTURIS);
  GET_JSON_VALUES_RETURN_NULL_ONERROR(json);
  KEY_VALUE_VARS(client_id, client_secret, scope, flow, redirect_uris);
  struct clientConfig* p = secAlloc(sizeof(struct clientConfig));
  p->client_id           = _client_id;
  p->client_secret       = _client_secret;
  p->scope               = _scope;
  p->flows               = JSONArrayStringToList(_flow);
  p->redirect_uris       = JSONArrayStringToList(_redirect_uris);
  secFree(_flow);
  return p;
}

struct issuerConfig* getIssuerConfigFromJSON(const cJSON* json) {
  if (NULL == json) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  INIT_KEY_VALUE(
      AGENT_KEY_ISSUERURL, OIDC_KEY_ISSUER, AGENT_KEY_CONFIG_ENDPOINT,
      AGENT_KEY_CERTPATH, OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT,
      AGENT_KEY_OAUTH, CONFIG_KEY_LEGACYAUDMODE, AGENT_KEY_PUBCLIENT,
      AGENT_KEY_MANUAL_CLIENT_REGISTRATION_URI, AGENT_KEY_CONTACT,
      AGENT_KEY_PWSTORE, AGENT_KEY_DEFAULT_ACCOUNT, AGENT_KEY_ACCOUNTS);
  GET_JSON_VALUES_CJSON_RETURN_NULL_ONERROR(json);
  KEY_VALUE_VARS(issuer_url, issuer, config_endpoint, cert_path,
                 device_authorization_endpoint, oauth, legacy_aud_mode,
                 pub_client, manual_register, contact, store_pw,
                 default_account, accounts);
  struct issuerConfig* c = secAlloc(sizeof(struct issuerConfig));
  if (_issuer) {
    c->issuer = _issuer;
    secFree(_issuer_url);
  } else {
    c->issuer = _issuer_url;
  }
  c->configuration_endpoint        = _config_endpoint;
  c->device_authorization_endpoint = _device_authorization_endpoint;
  c->cert_path                     = _cert_path;
  c->manual_register               = _manual_register;
  c->contact                       = _contact;
  c->default_account               = _default_account;
  c->oauth                         = strToBit(_oauth);
  c->oauth_set                     = _oauth != NULL;
  c->store_pw                      = strToBit(_store_pw);
  c->store_pw_set                  = _store_pw != NULL;
  c->legacy_aud_mode               = strToBit(_legacy_aud_mode);
  c->pub_client                    = getClientConfigFromJSON(_pub_client);
  c->accounts                      = JSONArrayStringToList(_accounts);
  secFree(_oauth);
  secFree(_store_pw);
  secFree(_pub_client);
  secFree(_accounts);
  secFree(_legacy_aud_mode);
  return c;
}

cJSON* clientConfigToJSON(const struct clientConfig* p) {
  if (p == NULL) {
    return NULL;
  }
  cJSON* json =
      generateJSONObject(OIDC_KEY_CLIENTID, cJSON_String, p->client_id, NULL);
  jsonAddStringValue(json, OIDC_KEY_CLIENTSECRET, p->client_secret);
  jsonAddStringValue(json, OIDC_KEY_SCOPE, p->scope);
  if (p->flows) {
    cJSON* flows = stringListToJSONArray(p->flows);
    jsonAddJSON(json, IPC_KEY_FLOW, flows);
  }
  if (p->redirect_uris) {
    cJSON* redirect_uris = stringListToJSONArray(p->redirect_uris);
    jsonAddJSON(json, OIDC_KEY_REDIRECTURIS, redirect_uris);
  }
  return json;
}

cJSON* issuerConfigToJSON(const struct issuerConfig* c) {
  if (c == NULL) {
    return NULL;
  }
  cJSON* json =
      generateJSONObject(OIDC_KEY_ISSUER, cJSON_String, c->issuer ?: "", NULL);
  if (c->accounts) {
    cJSON* accounts = stringListToJSONArray(c->accounts);
    jsonAddJSON(json, AGENT_KEY_ACCOUNTS, accounts);
  }
  if (c->pub_client) {
    cJSON* pubclient = clientConfigToJSON(c->pub_client);
    jsonAddJSON(json, AGENT_KEY_PUBCLIENT, pubclient);
  }
  jsonAddStringValue(json, AGENT_KEY_CONFIG_ENDPOINT,
                     c->configuration_endpoint);
  jsonAddStringValue(json, AGENT_KEY_CERTPATH, c->cert_path);
  jsonAddStringValue(json, OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT,
                     c->device_authorization_endpoint);
  jsonAddStringValue(json, AGENT_KEY_MANUAL_CLIENT_REGISTRATION_URI,
                     c->manual_register);
  jsonAddStringValue(json, AGENT_KEY_CONTACT, c->contact);
  jsonAddStringValue(json, AGENT_KEY_DEFAULT_ACCOUNT, c->default_account);
  if (c->store_pw_set) {
    jsonAddBoolValue(json, AGENT_KEY_PWSTORE, c->store_pw);
  }
  if (c->oauth_set) {
    jsonAddBoolValue(json, AGENT_KEY_OAUTH, c->oauth);
  }
  if (c->legacy_aud_mode) {
    jsonAddBoolValue(json, CONFIG_KEY_LEGACYAUDMODE, c->legacy_aud_mode);
  }
  return json;
}

static cJSON* collection = NULL;

static void collect_handleObject(cJSON* j) {
  char*  iss    = getJSONValue(j, OIDC_KEY_ISSUER);
  cJSON* parent = cJSON_CreateObject();
  cJSON_AddItemReferenceToObject(parent, iss, j);
  secFree(iss);
  cJSON* tmp = jsonMergePatch(parent, collection);
  secFreeJson(parent);
  secFreeJson(collection);
  collection = tmp;
}

static void collectJSONIssuers(const char* json) {
  if (json == NULL) {
    return;
  }
  if (collection == NULL) {
    collection = cJSON_CreateObject();
  }
  cJSON* j = stringToJson(json);
  if (j == NULL) {
    return;
  }
  if (cJSON_IsObject(j)) {
    collect_handleObject(j);
  } else if (cJSON_IsArray(j)) {
    cJSON* jj = j->child;
    while (jj) {
      collect_handleObject(jj);
      jj = jj->next;
    }
  }
  secFreeJson(j);
}

static list_t* _issuers = NULL;

static int issuerConfig_matchByIssuerUrl(const struct issuerConfig* c1,
                                         const struct issuerConfig* c2) {
  return matchUrls(c1->issuer, c2->issuer);
}

static int issuerConfig_compByAccountCount(const struct issuerConfig* c1,
                                           const struct issuerConfig* c2) {
  // Since we want to order ascending we return 1 if c1<c2
  if (c1 == c2) {
    return 0;
  }
  if (c1 == NULL) {
    return 1;
  }
  if (c2 == NULL) {
    return -1;
  }
  if (c1->accounts == c2->accounts) {
    return 0;
  }
  if (c1->accounts == NULL) {
    return 1;
  }
  if (c2->accounts == NULL) {
    return -1;
  }
  unsigned int l1 = c1->accounts->len;
  unsigned int l2 = c2->accounts->len;
  if (l1 == l2) {
    return 0;
  }
  return l1 < l2 ? 1 : -1;
}

static list_t* assert_issuers() {
  if (_issuers == NULL) {
    _issuers        = list_new();
    _issuers->free  = (freeFunction)_secFreeIssuerConfig;
    _issuers->match = (matchFunction)issuerConfig_matchByIssuerUrl;
  }
  return _issuers;
}

static char* updateIssuerConfigFileFormat(char* content) {
  cJSON* iss_list_json = cJSON_CreateArray();
  char*  elem          = strtok(content, "\n");
  while (elem != NULL) {
    char* space = strchr(elem, ' ');
    if (space) {
      *space = '\0';
    }
    char*       iss             = elem;
    const char* default_account = space ? space + 1 : NULL;
    list_t*     accounts        = newListWithSingleValue(default_account);
    const struct issuerConfig this_config = {
        .issuer          = iss,
        .default_account = (char*)default_account,
        .accounts        = accounts,
    };
    cJSON_AddItemToArray(iss_list_json, issuerConfigToJSON(&this_config));
    secFreeList(accounts);
    elem = strtok(NULL, "\n");
  }
  secFree(content);
  char* new_content = jsonToString(iss_list_json);
  secFreeJson(iss_list_json);
  writeOidcFile(ISSUER_CONFIG_FILENAME, new_content);
  return new_content;
}

static void readIssuerConfigs() {
  char* content = readOidcFile(ISSUER_CONFIG_FILENAME);
  if (content && !isJSONArray(content)) {  // old config file
    content = updateIssuerConfigFileFormat(content);
  }
  collectJSONIssuers(content);
  secFree(content);

  char*   oidcIssuerConfDir = concatToOidcDir(ISSUER_CONFIG_DIRNAME);
  list_t* conf_list =
      getFileListForDir(oidcIssuerConfDir, ISSUER_CONFIG_DIRNAME);
  secFree(oidcIssuerConfDir);
  if (conf_list) {
    list_mergeSort(conf_list, (matchFunction)compareOidcFilesByDateModified);
    list_iterator_t* it = list_iterator_new(conf_list, LIST_HEAD);
    list_node_t*     node;
    while ((node = list_iterator_next(it))) {
      content = readOidcFile(node->val);
      collectJSONIssuers(content);
      secFree(content);
    }
    secFreeList(conf_list);
    list_iterator_destroy(it);
  }

  content = readFile(
#ifdef ANY_MSYS
      ETC_ISSUER_CONFIG_FILE()
#else
      ETC_ISSUER_CONFIG_FILE
#endif
  );
  collectJSONIssuers(content);
  secFree(content);

  const char* etc_iss_dir =
#ifdef ANY_MSYS
      ETC_ISSUER_CONFIG_DIR();
#else
      ETC_ISSUER_CONFIG_DIR;
#endif
  conf_list = getFileListForDir(etc_iss_dir, etc_iss_dir);
  if (conf_list) {
    list_iterator_t* it = list_iterator_new(conf_list, LIST_HEAD);
    list_node_t*     node;
    while ((node = list_iterator_next(it))) {
      content = readFile(node->val);
      collectJSONIssuers(content);
      secFree(content);
    }
    secFreeList(conf_list);
    list_iterator_destroy(it);
  }

  cJSON* item = collection->child;
  do {
    struct issuerConfig* issConfig = getIssuerConfigFromJSON(item);
    list_lpush(assert_issuers(), list_node_new(issConfig));
    item = item->next;
  } while (item);
  secFreeJson(collection);
  list_mergeSort(assert_issuers(),
                 (matchFunction)issuerConfig_compByAccountCount);
}

static list_t* issuers() {
  if (_issuers == NULL) {
    assert_issuers();
    readIssuerConfigs();
  }
  return _issuers;
}

list_t* getSuggestableIssuers() {
  list_t* suggestions   = list_new();
  suggestions->match    = (matchFunction)matchUrls;
  list_iterator_t* it   = list_iterator_new(issuers(), LIST_HEAD);
  list_node_t*     node = NULL;
  while ((node = list_iterator_next(it))) {
    struct issuerConfig* c = node->val;
    if (c) {
      list_addStringIfNotFound(suggestions, c->issuer);
    }
  }
  list_iterator_destroy(it);
  return suggestions;
}

list_node_t* getIssuerNode(const char* iss) {
  struct issuerConfig key = {.issuer = (char*)iss};
  return findInList(issuers(), &key);
}

const struct issuerConfig* getIssuerConfig(const char* iss) {
  const list_node_t* node = getIssuerNode(iss);
  return node ? node->val : NULL;
}

list_t* defaultRedirectURIs() {
  list_t* redirect_uris =
      createList(0, "http://localhost:8080", "http://localhost:4242",
                 "http://localhost:43985", NULL);
  redirect_uris->match = (matchFunction)strequal;
  return redirect_uris;
}

/**
 * @brief updates the issuer.config file.
 * Adds an account for an issuer
 * If the issuer url is not already in the issuer.config file, it will be added.
 * @param issuer_url the issuer url to be added
 * @param shortname of the account config
 */
void oidcp_updateIssuerConfigAdd(const char* issuer_url,
                                 const char* shortname) {
  if (issuer_url == NULL || shortname == NULL) {
    return;
  }
  list_node_t* node = getIssuerNode(issuer_url);
  if (node == NULL) {
    struct issuerConfig* c = secAlloc(sizeof(struct issuerConfig));
    c->issuer              = oidc_strcopy(issuer_url);
    c->accounts            = newListWithSingleValue(shortname);
    list_rpush(issuers(), list_node_new(c));
  } else {
    struct issuerConfig* c = node->val;
    if (c->accounts) {
      if (findInList(c->accounts, shortname)) {
        return;
      }
      list_addStringIfNotFound(c->accounts, (char*)shortname);
    } else {
      c->accounts = newListWithSingleValue(shortname);
    }
  }
  cJSON* iss_list_json =
      listToJSONArray(issuers(), (cJSON * (*)(void*)) issuerConfigToJSON);
  char* new_content = jsonToString(iss_list_json);
  secFreeJson(iss_list_json);
  writeOidcFile(ISSUER_CONFIG_FILENAME, new_content);
  secFree(new_content);
}

/**
 * @brief updates the issuer.config file.
 * Adds an account for an issuer
 * If the issuer url is not already in the issuer.config file, it will be added.
 * @param issuer_url the issuer url to be added
 * @param shortname of the account config
 */
void oidcp_updateIssuerConfigDelete(const char* issuer_url,
                                    const char* shortname) {
  if (issuer_url == NULL || shortname == NULL) {
    return;
  }
  list_node_t* node = getIssuerNode(issuer_url);
  if (node == NULL) {
    return;
  } else {
    struct issuerConfig* c = node->val;
    list_removeIfFound(c->accounts, (char*)shortname);
  }
  cJSON* iss_list_json =
      listToJSONArray(issuers(), (cJSON * (*)(void*)) issuerConfigToJSON);
  char* new_content = jsonToString(iss_list_json);
  secFreeJson(iss_list_json);
  writeOidcFile(ISSUER_CONFIG_FILENAME, new_content);
  secFree(new_content);
}

void oidcp_updateIssuerConfig(const char* action, const char* issuer,
                              const char* shortname) {
  if (strequal(action, INT_ACTION_VALUE_ADD)) {
    oidcp_updateIssuerConfigAdd(issuer, shortname);
  } else if (strequal(action, INT_ACTION_VALUE_REMOVE)) {
    oidcp_updateIssuerConfigDelete(issuer, shortname);
  }
}

const list_t* getUserClientFlows(const char* issuer_url) {
  const struct issuerConfig* iss = getIssuerConfig(issuer_url);
  if (iss == NULL) {
    return NULL;
  }
  const struct clientConfig* client = iss->user_client;
  if (client == NULL) {
    return NULL;
  }
  return client->flows;
}

const list_t* getPubClientFlows(const char* issuer_url) {
  const struct issuerConfig* iss = getIssuerConfig(issuer_url);
  if (iss == NULL) {
    return NULL;
  }
  const struct clientConfig* pub = iss->pub_client;
  if (pub == NULL) {
    return NULL;
  }
  return pub->flows;
}

char* getAccountInfos(list_t* loaded) {
  cJSON*           json = cJSON_CreateObject();
  list_iterator_t* it   = list_iterator_new(issuers(), LIST_HEAD);
  list_node_t*     node = NULL;
  while ((node = list_iterator_next(it))) {
    struct issuerConfig* c = node->val;
    if (c == NULL) {
      continue;
    }
    cJSON* issObj = cJSON_CreateObject();
    cJSON_AddBoolToObject(
        issObj, ACCOUNTINFO_KEY_HASPUBCLIENT,
        c->pub_client != NULL && c->pub_client->client_id != NULL);
    if (c->accounts) {
      cJSON*           accounts    = cJSON_CreateObject();
      list_iterator_t* accounts_it = list_iterator_new(c->accounts, LIST_HEAD);
      list_node_t*     accounts_node = NULL;
      while ((accounts_node = list_iterator_next(accounts_it))) {
        const char* shortname = accounts_node->val;
        if (shortname == NULL) {
          continue;
        }
        cJSON_AddBoolToObject(accounts, shortname,
                              findInList(loaded, shortname) != NULL);
      }
      list_iterator_destroy(accounts_it);
      cJSON_AddItemToObject(issObj, AGENT_KEY_ACCOUNTS, accounts);
    }
    cJSON_AddItemToObject(json, c->issuer, issObj);
  }
  list_iterator_destroy(it);
  char* json_str = jsonToStringUnformatted(json);
  secFreeJson(json);
  return json_str;
}

const char* getDefaultAccountConfigForIssuer(const char* issuer_url) {
  if (issuer_url == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  const struct issuerConfig* c = getIssuerConfig(issuer_url);
  if (c == NULL) {
    return NULL;
  }
  if (strValid(c->default_account)) {
    return c->default_account;
  }
  if (!listValid(c->accounts)) {
    return NULL;
  }
  list_node_t* firstAccount = list_at(c->accounts, 0);
  return firstAccount ? firstAccount->val : NULL;
}
