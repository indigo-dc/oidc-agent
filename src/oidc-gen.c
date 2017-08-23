#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <argp.h>

#include "oidc-gen.h"
#include "provider.h"
#include "prompt.h"
#include "oidc_utilities.h"
#include "json.h"
#include "file_io.h"
#include "crypt.h"
#include "ipc.h"

#define CONF_ENDPOINT_SUFFIX ".well-known/openid-configuration"

#define OIDC_SOCK_ENV_NAME "OIDC_SOCK"

#define PROVIDER_CONFIG_FILENAME "issuer.config"

const char *argp_program_version = "oidc-gen 0.2.1";

const char *argp_program_bug_address = "<gabriel.zachmann@kit.edu>";

struct arguments {
  char* args[1];            /* provider */
  int delete;
  int debug;
  int verbose;
  char* file;
};

static struct argp_option options[] = {
  {"delete", 'd', 0, 0, "delete configuration for the given provider", 0},
  {"debug", 'g', 0, 0, "sets the log level to DEBUG", 0},
  {"verbose", 'v', 0, 0, "enables verbose mode. The stored data will be printed.", 0},
  {"file", 'f', "FILE", 0, "specifies file with client config", 0},
  {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key)
  {
    case 'd':
      arguments->delete = 1;
      break;
    case 'g':
      arguments->debug = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'f':
      arguments->file = arg;
      break;
    case ARGP_KEY_ARG:
      if(state->arg_num >= 1) {
        argp_usage(state);
      }
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if(state->arg_num < 1 && arguments->delete) {
        argp_usage (state);
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static char args_doc[] = "[SHORT_NAME] | SHORT_NAME -d";

static char doc[] = "oidc-gen -- A tool for generating oidc provider configuration which can be used by oidc-add";

static struct argp argp = {options, parse_opt, args_doc, doc};


static struct oidc_provider* provider = NULL;


char* encryptionPassword = NULL;

int main(int argc, char** argv) {
  openlog("oidc-gen", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));

  struct arguments arguments;
  arguments.delete = 0;
  arguments.debug = 0;
  arguments.args[0] = NULL;
  arguments.file = NULL;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }
  if(arguments.delete) {
    if(!oidcFileDoesExist(arguments.args[0])) {
      fprintf(stderr, "No provider with that shortname configured\n");
      exit(EXIT_FAILURE);
    } 
    struct oidc_provider* loaded_p = NULL;
    while(NULL==loaded_p) {
      encryptionPassword = promptPassword("Enter encryption Password: ");
      loaded_p = decryptProvider(arguments.args[0], encryptionPassword);
    }
    char* json = providerToJSON(*loaded_p);
    freeProvider(loaded_p);
    if(removeOidcFile(arguments.args[0])==0) {
      printf("Successfully deleted provider configuration.\n");
    }

    struct connection con = {0,0,0};
    if(ipc_init(&con, OIDC_SOCK_ENV_NAME, 0)!=0) {
      exit(EXIT_FAILURE);
    }
    if(ipc_connect(con)<0) {
      printf("Could not connect to oicd\n");
      exit(EXIT_FAILURE);
    }
    ipc_write(*(con.sock), REQUEST_CONFIG, "remove", json);
    clearFreeString(json);
    char* res = ipc_read(*(con.sock));
    ipc_close(&con);
    if(NULL==res) {
      printf("An unexpected error occured. It's seems that oidc-agent has stopped.\n That's not good.");
      exit(EXIT_FAILURE);
    }

    struct key_value pairs[2];
    pairs[0].key = "status";
    pairs[1].key = "error";
    if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
      printf("Could not decode json: %s\n", res);
      printf("This seems to be a bug. Please hand in a bug report.\n");
      clearFreeString(res);
      exit(EXIT_FAILURE);
    }
    clearFreeString(res);
    if(strcmp(pairs[0].value, "success")==0 || strcmp(pairs[1].value, "provider not loaded")==0) {
      printf("The generated provider was successfully removed from oidc-agent. You don't have to run oidc-add.\n");
      clearFreeString(pairs[0].value);
      exit(EXIT_SUCCESS);
    }
    if(pairs[1].value!=NULL) {
      printf("Error: %s\n", pairs[1].value);
      printf("The provider was not removed from oidc-agent. Please run oidc-add with -r to try it again.\n");
      clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
      exit(EXIT_FAILURE);
    }

  } 

  initCrypt();
  provider = genNewProvider(arguments.args[0]);
  char* json = providerToJSON(*provider);
  struct connection con = {0,0,0};
  if(ipc_init(&con, OIDC_SOCK_ENV_NAME, 0)!=0) {
    exit(EXIT_FAILURE);
  }
  if(ipc_connect(con)<0) {
    printf("Could not connect to oicd\n");
    exit(EXIT_FAILURE);
  }
  ipc_write(*(con.sock), REQUEST_CONFIG, "gen", json);
  char* res = ipc_read(*(con.sock));
  ipc_close(&con);
  if(NULL==res) {
    printf("An unexpected error occured. It's seems that oidc-agent has stopped.\n That's not good.");
    exit(EXIT_FAILURE);
  }

  struct key_value pairs[4];
  pairs[0].key = "status";
  pairs[1].key = "refresh_token";
  pairs[2].key = "token_endpoint";
  pairs[3].key = "error";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printf("Could not decode json: %s\n", res);
    printf("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    saveExit(EXIT_FAILURE);
  }
  clearFreeString(res);
  if(pairs[3].value!=NULL) {
    printf("Error: %s\n", pairs[3].value);
    clearFreeString(pairs[3].value); clearFreeString(pairs[2].value); clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
    saveExit(EXIT_FAILURE);
  }
  if(pairs[2].value!=NULL) {
    provider_setTokenEndpoint(provider, pairs[2].value);
  } else {
    fprintf(stderr, "Error: response does not contain token_endpoint\n");
  }
  if(pairs[1].value!=NULL) {
    provider_setRefreshToken(provider, pairs[1].value);
  }
  printf("%s\n", pairs[0].value);
  if(strcmp(pairs[0].value, "success")==0) {
    printf("The generated provider was successfully added to oidc-agent. You don't have to run oidc-add.\n");
  }
  clearFreeString(pairs[0].value);

  // remove username and password from config
  provider_setUsername(provider, NULL);
  provider_setPassword(provider, NULL);
  clearFreeString(json);
  json = providerToJSON(*provider);

  // if issuer isn't already in issuer.config than add it
  char* issuers = readOidcFile(PROVIDER_CONFIG_FILENAME);
  if(strcasestr(issuers, provider_getIssuer(*provider))==NULL) {
    char* tmp = calloc(sizeof(char), snprintf(NULL, 0, "%s%s", issuers, provider_getIssuer(*provider))+1);
    if (tmp==NULL) {
      fprintf(stderr, "calloc failed %m");
      saveExit(EXIT_FAILURE);
    }
    sprintf(tmp, "%s%s", issuers, provider_getIssuer(*provider));
    clearFreeString(issuers);
    issuers = tmp;
    writeOidcFile(PROVIDER_CONFIG_FILENAME, issuers);
  }
  clearFreeString(issuers);

  if(arguments.verbose) {
    printf("The following data will be saved encrypted:\n%s\n", json);
  }

  //encrypt
  do {
    char* input = promptPassword("Enter encrpytion password%s: ", encryptionPassword ? " [***]" : "");
    if(encryptionPassword && !isValid(input)) { // use same encrpytion password
      clearFreeString(input);
      break;
    } else {
      if(encryptionPassword) {
        clearFreeString(encryptionPassword);
      }
      encryptionPassword = input;
    }
    char* confirm = promptPassword("Confirm encryption Password: ");
    if(strcmp(encryptionPassword, confirm)!=0) {
      printf("Encryption passwords did not match.\n");
      if(confirm) {
        clearFreeString(confirm); 
      }
      if(encryptionPassword) {
        clearFreeString(encryptionPassword);
        encryptionPassword = NULL;
      }
      continue;
    }
    clearFreeString(confirm);
  } while(encryptionPassword==NULL);
  char* toWrite = encryptProvider(json, encryptionPassword);
  clearFreeString(json);
  if(encryptionPassword) {
    clearFreeString(encryptionPassword);
  }

  writeOidcFile(provider->name, toWrite);
  clearFreeString(toWrite);
  saveExit(EXIT_SUCCESS);
  return EXIT_FAILURE;
}

void promptAndSet(char* prompt_str, void (*set_callback)(struct oidc_provider*, char*), char* (*get_callback)(struct oidc_provider), int passPrompt, int optional) {
  char* input = NULL;
  do {
    if(passPrompt) {
      input = promptPassword(prompt_str, get_callback(*provider) ? " [***]" : "");
    } else {
      input = prompt(prompt_str, get_callback(*provider) ? " [" : "", get_callback(*provider) ? get_callback(*provider) : "", get_callback(*provider) ? "]" : "");
    }
    if(isValid(input)) {
      set_callback(provider, input);
    } else {
      clearFreeString(input);
    }
    if(optional) {
      break;
    }
  } while(!isValid(get_callback(*provider)));
}

void promptAndSetIssuer() {
  if(!oidcFileDoesExist(PROVIDER_CONFIG_FILENAME)) {
    promptAndSet("Issuer%s%s%s: ", provider_setIssuer, provider_getIssuer, 0, 0);
  } else {
    char* fileContent = readOidcFile(PROVIDER_CONFIG_FILENAME);
    char* s = fileContent;
    int i;
    for (i=0; s[i]; s[i]=='\n' ? i++ : *s++); // counts the lines in the file
    int size = i++;
    char* providers[size];
    providers[0] = strtok(fileContent, "\n");
    for(i=1; i<size; i++) {
      providers[i] = strtok(NULL, "\n");
      if(providers[i]==NULL) {
        size=i;
      }
    }
    char* fav = providers[0];
    for(i=size-1; i>=0; i--) {
      if(strcasestr(providers[i], provider_getName(*provider))) { // if the short name is a substring of the issuer it's likely that this is the fav issuer
        fav = providers[i];
      }
    }
    if(isValid(provider_getIssuer(*provider))) {
      fav = provider_getIssuer(*provider);
    }
prompting:
    for(i=0; i<size; i++)
      printf("[%d] %s\n", i+1, providers[i]); // printed indices starts at 1 for non nerds
    char* input = prompt("Issuer [%s]: ", fav);
    char* iss = NULL;
    if(!isValid(input)) {
      iss = calloc(sizeof(char), strlen(fav)+1);
      strcpy(iss, fav);
      clearFreeString(input);
    } else if (isdigit(*input)) {
      i = atoi(input);
      clearFreeString(input);
      if(i>size || i<1) {
        printf("input out of bound\n");
        goto prompting;
      }
      i--; // printed indices starts at 1 for non nerds
      iss = calloc(sizeof(char), strlen(providers[i])+1);
      strcpy(iss, providers[i]);
    } else {
      iss = input;
    }
    clearFreeString(fileContent);
    provider_setIssuer(provider, iss);


  }
  int issuer_len = strlen(provider_getIssuer(*provider));
  if(provider_getIssuer(*provider)[issuer_len-1]!='/') {
    void* tmp = realloc(provider_getIssuer(*provider), issuer_len+1+1);   
    if(NULL==tmp) {
      printf("realloc failed\n");
      exit(EXIT_FAILURE);
    }
    provider->issuer = tmp; // don't use provider_setIssuer here, because the free in it would double free because of realloc
    provider_getIssuer(*provider)[issuer_len] = '/';
    issuer_len = strlen(provider_getIssuer(*provider));
    provider_getIssuer(*provider)[issuer_len] = '\0';
  }

  provider_setConfigEndpoint(provider, calloc(sizeof(char), issuer_len + strlen(CONF_ENDPOINT_SUFFIX) + 1));
  sprintf(provider_getConfigEndpoint(*provider), "%s%s", provider_getIssuer(*provider), CONF_ENDPOINT_SUFFIX);

}

struct oidc_provider* genNewProvider(const char* short_name) {
  provider = calloc(sizeof(struct oidc_provider), 1);
  while(!isValid(provider_getName(*provider))) {
    if(short_name) {
      char* name = calloc(sizeof(char), strlen(short_name)+1);
      strcpy(name, short_name);
      provider_setName(provider, name);

      if(oidcFileDoesExist(provider_getName(*provider))) {
        struct oidc_provider* loaded_p = NULL;
        while(NULL==loaded_p) {
          clearFreeString(encryptionPassword);
          encryptionPassword = promptPassword("Enter encryption Password: ");
          loaded_p = decryptProvider(provider_getName(*provider), encryptionPassword);
        }
        freeProvider(provider);
        provider = loaded_p;
        goto prompting;
      } else {
        printf("No provider exists with this short name. Creating new configuration ...\n");
        goto prompting;
      }
    }
    provider_setName(provider, prompt("Enter short name for the provider to configure: ")); 
    if(!isValid(provider_getName(*provider))) {
      continue;
    }
    if(oidcFileDoesExist(provider_getName(*provider))) {
      char* res = prompt("A provider with this short name is already configured. Do you want to edit the configuration? [yes/no/quit]: ");
      if(strcmp(res, "yes")==0) {
        clearFreeString(res);
        struct oidc_provider* loaded_p = NULL;
        while(NULL==loaded_p) {
          encryptionPassword = promptPassword("Enter encryption Password: ");
          loaded_p = decryptProvider(provider_getName(*provider), encryptionPassword);
        }
        freeProvider(provider);
        provider = loaded_p;
        break;
      } else if(strcmp(res, "quit")==0) {
        exit(EXIT_SUCCESS);
      } else {
        clearFreeString(res);
        provider_setName(provider, NULL);
        continue; 
      }
    }
  }
prompting:
  if(!isValid(provider_getCertPath(*provider))) {
    unsigned int i;
    for(i=0; i<sizeof(possibleCertFiles)/sizeof(*possibleCertFiles); i++) {
      if(fileDoesExist(possibleCertFiles[i])) {
        char* cert_path = calloc(sizeof(char), strlen(possibleCertFiles[i])+1);
        strcpy(cert_path, possibleCertFiles[i]);
        provider_setCertPath(provider, cert_path);
        break;
      }
    }
  }
  promptAndSet("Cert Path%s%s%s: ", provider_setCertPath, provider_getCertPath, 0, 0);
  promptAndSetIssuer();
  promptAndSet("Client_id%s%s%s: ", provider_setClientId, provider_getClientId, 0, 0);
  promptAndSet("Client_secret%s%s%s: ", provider_setClientSecret, provider_getClientSecret, 0, 0);
  promptAndSet("Refresh token%s%s%s: ", provider_setRefreshToken, provider_getRefreshToken, 0, 1);
  promptAndSet("Username%s%s%s: ", provider_setUsername, provider_getUsername, 0, isValid(provider_getRefreshToken(*provider)));
  promptAndSet("Password%s: ", provider_setPassword, provider_getPassword, 1, isValid(provider_getRefreshToken(*provider)));

  return provider;
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
  char* write_it = calloc(sizeof(char), snprintf(NULL, 0, fmt, cipher_len, salt_hex, nonce_hex, cipher_hex)+1);
  sprintf(write_it, fmt, cipher_len, salt_hex, nonce_hex, cipher_hex);
  clearFreeString(cipher_hex);
  return write_it;
}

