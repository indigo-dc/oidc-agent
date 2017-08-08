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

static struct oidc_provider* provider = NULL;

int main(/* int argc, char** argv */) {
  provider = genNewProvider();
  char* json = providerToJSON(*provider);
  // TODO print config and prompt confirmation
  struct connection con = {};
  if(ipc_init(&con, NULL, "OIDC_GEN_SOCK", 0)!=0)
    exit(EXIT_FAILURE);
  if(ipc_connect(con)<0) {
    printf("Could not connect to oicd\n");
    exit(EXIT_FAILURE);
  }
  ipc_write(*(con.sock), json);
  char* res = ipc_read(*(con.sock));
  ipc_close(&con);
  if(NULL==res) {
    printf("An unexpected error occured. It's seems that oidcd has stopped.\n That's not good.");
    exit(EXIT_FAILURE);
  }

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

void promptAndSet(char* prompt_str, void (*set_callback)(struct oidc_provider*, char*), char* (*get_callback)(struct oidc_provider), int passPrompt, int optional) {
  char* input = NULL;
  do {
    if(passPrompt)
      input = promptPassword(prompt_str, get_callback(*provider) ? " [***]" : "");
    else
      input = prompt(prompt_str, get_callback(*provider) ? " [" : "", get_callback(*provider) ? get_callback(*provider) : "", get_callback(*provider) ? "]" : "");
    if(isValid(input))
      set_callback(provider, input);
    else
      free(input);
    if(optional) {
      break;
    }
  } while(!isValid(get_callback(*provider)));
}

void promptAndSetIssuer() {
  promptAndSet("Issuer%s%s%s: ", provider_setIssuer, provider_getIssuer, 0, 0);
  int issuer_len = strlen(provider_getIssuer(*provider));
  if(provider_getIssuer(*provider)[issuer_len-1]!='/') {
    provider->issuer = realloc(provider_getIssuer(*provider), issuer_len+1+1); // don't use provider_setIssuer here, because of the free
    if(NULL==provider_getIssuer(*provider)) {
      printf("realloc failed\n");
      exit(EXIT_FAILURE);
    }
    provider_getIssuer(*provider)[issuer_len] = '/';
    issuer_len = strlen(provider_getIssuer(*provider));
    provider_getIssuer(*provider)[issuer_len] = '\0';
  }

  printf("Issuer is now: %s\n", provider_getIssuer(*provider));

  provider_setConfigEndpoint(provider, calloc(sizeof(char), issuer_len + strlen(CONF_ENDPOINT_SUFFIX) + 1));
  sprintf(provider_getConfigEndpoint(*provider), "%s%s", provider_getIssuer(*provider), CONF_ENDPOINT_SUFFIX);

  printf("configuration_endpoint is now: %s\n", provider_getConfigEndpoint(*provider));
  provider_setTokenEndpoint(provider, getTokenEndpoint(provider_getConfigEndpoint(*provider)));
  if(NULL==provider_getTokenEndpoint(*provider)) {
    printf("Could not get token endpoint. Please fix issuer!\n");
    promptAndSetIssuer();
    return;
  }

  printf("token_endpoint is now: %s\n", provider_getTokenEndpoint(*provider));

}

struct oidc_provider* genNewProvider() {
  provider = calloc(sizeof(struct oidc_provider), 1);
  while(!isValid(provider_getName(*provider))) {
    provider_setName(provider, prompt("Enter short name for the provider to configure: ")); 
    if(oidcFileDoesExist(provider_getName(*provider))) {
      char* res = prompt("A provider with this short name is already configured. Do you want to edit the configuration? [yes/no/quit]: ");
      if(strcmp(res, "yes")==0) {
        //TODO
        free(res);
        struct oidc_provider* loaded_p = NULL;
        while(NULL==loaded_p) {
          char* encryptionPassword = promptPassword("Enter the encryption Password: ");
          loaded_p = decryptProvider(provider_getName(*provider), encryptionPassword);
          free(encryptionPassword);
        }
        freeProvider(provider);
        provider = loaded_p;
        break;
      }else if(strcmp(res, "quit")==0) {
        exit(EXIT_SUCCESS);
      } else {
        free(res);
        provider_setName(provider, NULL);
        continue; 
      }
    }
  }
  promptAndSetIssuer();
  promptAndSet("Client_id%s%s%s: ", provider_setClientId, provider_getClientId, 0, 0);
  promptAndSet("Client_secret%s%s%s: ", provider_setClientSecret, provider_getClientSecret, 0, 0);
  promptAndSet("Refresh token%s%s%s: ", provider_setRefreshToken, provider_getRefreshToken, 0, 1);
  promptAndSet("Username%s%s%s: ", provider_setUsername, provider_getUsername, 0, isValid(provider_getRefreshToken(*provider)));
  promptAndSet("Password%s: ", provider_setPassword, provider_getPassword, 1, isValid(provider_getRefreshToken(*provider)));

  return provider;
}

/** @fn char* getTokenEndpoint(const char* configuration_endpoint)
 * @brief retrieves provider config from the configuration_endpoint
 * @note the configuration_endpoint has to set prior
 * @param index the index identifying the provider
 */
char* getTokenEndpoint(const char* configuration_endpoint) {
  char* res = httpsGET(configuration_endpoint);
  if(NULL==res)
    return NULL;
  char* token_endpoint = getJSONValue(res, "token_endpoint");
  free(res);
  if (isValid(token_endpoint)) {
    return token_endpoint;
  } else {
    free(token_endpoint);
    printf("Could not get token_endpoint from the configuration endpoint.\nIf you currently have network issues, please try again later.\nOtherwise reconfigure with correct issuer.");
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

