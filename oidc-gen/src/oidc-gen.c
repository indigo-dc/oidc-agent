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
#include "../../src/crypt.h"
#include "../../src/ipc.h"

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

struct oidc_provider* provider = NULL;

int main(/* int argc, char** argv */) {
  printf("ECHO %s\n", getenv("OIDC_GEN_SOCK"));
  provider = genNewProvider();
  char* json = providerToJSON(*provider);
  // TODO print config and prompt confirmation
  struct connection con = {};
  printf("ECHO %s\n", getenv("OIDC_GEN_SOCK"));
  if(ipc_init(&con, NULL, "OIDC_GEN_SOCK", 0)!=0)
    exit(EXIT_FAILURE);
  if(ipc_connect(con)<0) {
    printf("Could not connect to oicd\n");
    exit(EXIT_FAILURE);
  }
  ipc_write(*(con.sock), json);
  char* res = ipc_read(*(con.sock));
  ipc_close(&con);

  struct key_value pairs[3];
  pairs[0].key = "status";
  pairs[1].key = "refresh_token";
  pairs[2].key = "error";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printf("Could not decode json: %s\n", res);
    printf("This seems to be a bug. Please hand in a bug report.\n");
    free(res);
    saveExit(EXIT_FAILURE);
  }
  free(res);
  if(pairs[2].value!=NULL) {
    printf("Error: %s\n", pairs[2].value);
    free(pairs[2].value); free(pairs[1].value); free(pairs[0].value);
    saveExit(EXIT_FAILURE);
  }
  if(pairs[1].value!=NULL) {
    provider_setRefreshToken(provider, pairs[1].value);
    free(json);
    json = providerToJSON(*provider);
  }
  printf("%s\n", pairs[0].value);
  free(pairs[0].value);

  char* encryptionPassword = promptPassword("Enter encrpytion password: ");
  char* toWrite = encryptProvider(json, encryptionPassword);
  free(json);
  free(encryptionPassword);
  writeOidcFile(provider->name, toWrite);
  free(toWrite);
  saveExit(EXIT_SUCCESS);
  return EXIT_FAILURE;
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

struct oidc_provider* genNewProvider() {
  //TODO validation checks, empty ,NULL, http
  struct oidc_provider* provider = calloc(sizeof(struct oidc_provider), 1);
  while(!isValid(provider_getName(*provider))) {
    provider_setName(provider, prompt("Enter short name for the provider to configure: ")); 
    if(oidcFileDoesExist(provider_getName(*provider))) {
      char* res = prompt("A provider with this short name is already configured. Do you want to edit the configuration? [yes/no/quit]: ");
      if(strcmp(res, "yes")==0) {
        //TODO
        free(res);
        char* encryptionPassword = promptPassword("Enter the encryption Password: ");
        struct oidc_provider* loaded_p = decryptProvider(provider_getName(*provider), encryptionPassword);
        free(encryptionPassword);
        freeProvider(provider);
        provider = loaded_p;
        break;
      } else if(strcmp(res, "quit")==0) {
          exit(EXIT_SUCCESS);
          } else {
        free(res);
        provider_setName(provider, NULL);
        continue; 
      }
    }
  }
  char* iss = prompt("Issuer: [%s]", provider_getIssuer(*provider) ? provider_getIssuer(*provider) : "");
  if(isValid(iss)) {
    provider_setIssuer(provider, iss);
  } else {
    free(iss);
  }
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


char* encryptProvider(const char* json, const char* password) {
  char salt_hex[2*SALT_LEN+1] = {0};
  char nonce_hex[2*NONCE_LEN+1] = {0};
  unsigned long cipher_len = strlen(json) + MAC_LEN;

  char* cipher_hex = encrypt((unsigned char*)json, password, nonce_hex, salt_hex);
  char* fmt = "%lu:%s:%s:%s";
  char* write_it = calloc(sizeof(char), strlen(cipher_hex)+strlen(salt_hex)+strlen(nonce_hex)+strlen(fmt)+snprintf(NULL, 0, "%lu", cipher_len));
  sprintf(write_it, fmt, cipher_len, salt_hex, nonce_hex, cipher_hex);
  free(cipher_hex);
  return write_it;
}

