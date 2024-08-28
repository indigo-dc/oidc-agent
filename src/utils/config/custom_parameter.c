#include "custom_parameter.h"

#include <stddef.h>

#include "defines/agent_values.h"
#include "defines/settings.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/json.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

struct custom_parameter_tuple {
  char* parameter;
  char* value;
  char* issuer;
  char* account;
  char* request;
};

int match_custom_parameter_tuples(struct custom_parameter_tuple* a,
                                  struct custom_parameter_tuple* b) {
  if (a == NULL && b == NULL) {
    return 1;
  }
  if (a == NULL || b == NULL) {
    return 0;
  }
  return strequal(a->parameter, b->parameter) &&
         strequal(a->request, b->request) && strequal(a->issuer, b->issuer) &&
         strequal(a->account, b->account);
}

void _secFreeCustomParameterTuple(struct custom_parameter_tuple* p) {
  if (p == NULL) {
    return;
  }
  secFree(p->parameter);
  secFree(p->value);
  secFree(p->issuer);
  secFree(p->account);
  secFree(p->request);
  secFree(p);
}

char* tmp_file_value = NULL;

const char* getValue(const struct custom_parameter_tuple* p) {
  const char* v = p->value;
  if (!strValid(v)) {
    return NULL;
  }
  secFree(tmp_file_value);
  switch (v[0]) {
    case '$': return (getenv(v + 1));
    case '/': tmp_file_value = getLineFromFile(v); return tmp_file_value;
    default: return (v);
  }
}

list_t* file_values        = NULL;
list_t* _custom_parameters = NULL;

list_t* parseCustomParametersObject(cJSON* j) {
  INIT_KEY_VALUE(CUSTOMPARAMETERS_KEY_PARAMETER, CUSTOMPARAMETERS_KEY_VALUE,
                 CUSTOMPARAMETERS_KEY_ISSUERS, CUSTOMPARAMETERS_KEY_ACCOUNTS,
                 CUSTOMPARAMETERS_KEY_REQUESTS);
  GET_JSON_VALUES_CJSON_RETURN_NULL_ONERROR(j);
  KEY_VALUE_VARS(parameter, value, issuers, accounts, requests);
  list_t* requests = JSONArrayStringToList(_requests);
  if (requests == NULL) {
    SEC_FREE_KEY_VALUES();
    return NULL;
  }
  list_t* issuers  = JSONArrayStringToList(_issuers);
  list_t* accounts = JSONArrayStringToList(_accounts);
  if (issuers == NULL && accounts == NULL) {
    SEC_FREE_KEY_VALUES();
    secFreeList(requests);
    return NULL;
  }
  list_t* tuples = list_new();
  tuples->free   = (freeFunction)_secFreeCustomParameterTuple;
  tuples->match  = (matchFunction)match_custom_parameter_tuples;

  list_iterator_t* requests_it = list_iterator_new(requests, LIST_HEAD);
  list_node_t*     requests_node;
  while ((requests_node = list_iterator_next(requests_it))) {
    if (issuers) {
      list_iterator_t* issuers_it = list_iterator_new(issuers, LIST_HEAD);
      list_node_t*     issuers_node;
      while ((issuers_node = list_iterator_next(issuers_it))) {
        struct custom_parameter_tuple* p =
            secAlloc(sizeof(struct custom_parameter_tuple));
        p->parameter = oidc_strcopy(_parameter);
        p->value     = oidc_strcopy(_value);
        p->request   = oidc_strcopy(requests_node->val);
        p->issuer    = oidc_strcopy(issuers_node->val);
        list_rpush(tuples, list_node_new(p));
      }
      list_iterator_destroy(issuers_it);
    }
    if (accounts) {
      list_iterator_t* accounts_it = list_iterator_new(accounts, LIST_HEAD);
      list_node_t*     accounts_node;
      while ((accounts_node = list_iterator_next(accounts_it))) {
        struct custom_parameter_tuple* p =
            secAlloc(sizeof(struct custom_parameter_tuple));
        p->parameter = oidc_strcopy(_parameter);
        p->value     = oidc_strcopy(_value);
        p->request   = oidc_strcopy(requests_node->val);
        p->account   = oidc_strcopy(accounts_node->val);
        list_rpush(tuples, list_node_new(p));
      }
      list_iterator_destroy(accounts_it);
    }
  }
  list_iterator_destroy(requests_it);
  secFreeList(accounts);
  secFreeList(issuers);
  secFreeList(requests);
  SEC_FREE_KEY_VALUES();
  return tuples;
}

void parseCustomParametersFileContent(const char* content) {
  if (content == NULL) {
    return;
  }
  cJSON* json = stringToJson(content);
  if (json == NULL) {
    return;
  }
  if (!cJSON_IsArray(json)) {
    logger(ERROR, "malformed " CUSTOM_PARAMETERS_FILENAME);
    secFreeJson(json);
    return;
  }
  size_t current_len = _custom_parameters->len;
  cJSON* j           = json->child;
  while (j) {
    list_t* parameters = parseCustomParametersObject(j);
    if (parameters == NULL) {
      j = j->next;
      continue;
    }
    parameters->free = NULL;  // We'll manually free each node IF we do not add
                              // it to the other list.
    list_iterator_t* it = list_iterator_new(parameters, LIST_HEAD);
    list_node_t*     node;
    while ((node = list_iterator_next(it))) {
      struct custom_parameter_tuple* p = node->val;
      if (findInListUntil(_custom_parameters, current_len, p)) {
        _secFreeCustomParameterTuple(p);
      } else {
        list_rpush(_custom_parameters, list_node_new(p));
      }
    }
    list_iterator_destroy(it);
    secFreeList(parameters);
    j = j->next;
  }
  secFreeJson(json);
}

list_t* custom_parameters() {
  if (_custom_parameters != NULL) {
    return _custom_parameters;
  }
  _custom_parameters        = list_new();
  _custom_parameters->free  = (freeFunction)_secFreeCustomParameterTuple;
  _custom_parameters->match = (matchFunction)match_custom_parameter_tuples;
  char* agent_dir_content   = readOidcFile(CUSTOM_PARAMETERS_FILENAME);
  char* etc_content         = readFile(
#ifdef ANY_MSYS
      ETC_CUSTOM_PARAMETERS_FILE()
#else
      ETC_CUSTOM_PARAMETERS_FILE
#endif
  );
  parseCustomParametersFileContent(agent_dir_content);
  parseCustomParametersFileContent(etc_content);
  secFree(agent_dir_content);
  secFree(etc_content);
  return _custom_parameters;
}

void _addCustomParameters(list_t* request_params, list_t* custom_parameters,
                          const struct oidc_account* account,
                          const char*                request_type) {
  secFreeList(file_values);
  file_values        = list_new();
  file_values->match = (matchFunction)strequal;
  file_values->free  = _secFree;

  list_iterator_t* it = list_iterator_new(custom_parameters, LIST_HEAD);
  list_node_t*     node;
  while ((node = list_iterator_next(it))) {
    struct custom_parameter_tuple* p = node->val;
    if (!strequal(request_type, p->request)) {
      continue;
    }
    if (!strequal(p->issuer, account_getIssuerUrl(account)) &&
        !strequal(p->account, account_getName(account))) {
      continue;
    }
    list_rpush(request_params, list_node_new(p->parameter));
    list_rpush(request_params, list_node_new((void*)getValue(p)));
    if (tmp_file_value != NULL) {
      list_rpush(file_values, list_node_new(tmp_file_value));
      tmp_file_value = NULL;
    }
  }
  list_iterator_destroy(it);
}

void addCustomParameters(list_t*                    request_params,
                         const struct oidc_account* account,
                         const char*                request_type) {
  _addCustomParameters(request_params, custom_parameters(), account,
                       request_type);
}