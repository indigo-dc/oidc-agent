#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "oidc-gen.h"
#include "../../src/provider.h"
#include "../../src/prompt.h"
#include "../../src/http.h"
#include "../../src/oidc_string.h"
#include "../../src/json.h"
#include "../../src/file_io.h"

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

struct oidc_provider* provider = NULL;

int main(int argc, char** argv) {
  provider = genNewProvider();
  char* json = providerToJSON(*provider);
  // TODO print config and prompt confirmation
  // TODO init run, get refresh_token
  // TODO encrypt it
  printf("%s\n", json);
  writeOidcFile(provider->name, json);

  saveExit(EXIT_SUCCESS);
}

struct oidc_provider* genNewProvider() {
  //TODO validation checks, empty ,NULL, http
  struct oidc_provider* provider = calloc(sizeof(struct oidc_provider), 1);
  provider->name = prompt("Enter short name for the provider to configure: "); //TODO check if it is already taken

  provider->issuer = prompt("Issuer: ");
  int issuer_len = strlen(provider->issuer);
  if(provider->issuer[issuer_len-1]!='/') {
    provider->issuer = realloc(provider->issuer, issuer_len+1+1);
    if(NULL==provider->issuer) {
      printf("realloc failed\n");
      exit(EXIT_FAILURE);
    }
    provider->issuer[issuer_len] = '/';
    issuer_len = strlen(provider->issuer);
    provider->issuer[issuer_len] = '\0';
  }

  printf("Issur is now: %s\n", provider->issuer);

  provider->configuration_endpoint = calloc(sizeof(char), issuer_len + strlen(CONF_ENDPOINT_SUFFIX) + 1);
  sprintf(provider->configuration_endpoint, "%s%s", provider->issuer, CONF_ENDPOINT_SUFFIX);

  printf("configuration_endpoint is now: %s\n", provider->configuration_endpoint);
  provider->token_endpoint = getTokenEndpoint(provider->configuration_endpoint);

  printf("token_endpoint is now: %s\n", provider->token_endpoint);
  provider->client_id = prompt("Client_id: ");

  provider->client_secret = prompt("Client_secret: ");

  provider->refresh_token = prompt("Refresh token: ");

  provider->username = prompt("Username: ");

  provider->password = promptPassword("Password: ");

  return provider;
}

void freeProvider(struct oidc_provider* p) {
  free(p->name);
  free(p->issuer);
  free(p->configuration_endpoint);
  free(p->token_endpoint);
  free(p->client_id);
  free(p->client_secret);
  free(p->username);
  free(p->password);
  free(p->refresh_token);
  free(p);
}

/** @fn char* getTokenEndpoint(const char* configuration_endpoint)
 * @brief retrieves provider config from the configuration_endpoint
 * @note the configuration_endpoint has to set prior
 * @param index the index identifying the provider
 */
char* getTokenEndpoint(const char* configuration_endpoint) {
  char* res = httpsGET(configuration_endpoint);
  char* token_endpoint = getJSONValue(res, "token_endpoint");
  free(res);
  if (isValid(token_endpoint)) {
    return token_endpoint;
  } else {
    free(token_endpoint);
    printf("Could not get token_endpoint from the configuration endpoint.\nIf you currently have network issues, please try again later.\nOtherwise reconfigure with correct issuer.");
    saveExit(EXIT_FAILURE);
    return NULL;
  }
}

void saveExit(int exitno) {
  freeProvider(provider);
  exit(exitno);
}

char* providerToJSON(struct oidc_provider p) {
  char* fmt = "{\n\"name\":\"%s\",\n\"issuer\":\"%s\",,\n\"configuration_endpoint\":\"%s\",\n\"token_endpoint\":\"%s\",\n\"client_id\":\"%s\",\n\"client_secret\":\"%s\",\n\"username\":\"%s\",\n\"password\":\"%s\",\n\"refresh_token\":\"%s\"\n}";
  char* p_json = calloc(sizeof(char), snprintf(NULL, 0, fmt, p.name, p.issuer, p.configuration_endpoint, p.token_endpoint, p.client_id, p.client_secret, p.username, p.password, p.refresh_token)+1);
  sprintf(p_json, fmt, p.name, p.issuer, p.configuration_endpoint, p.token_endpoint, p.client_id, p.client_secret, p.username, p.password, p.refresh_token);
  return p_json;
}

