#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "oidc-gen.h"
#include "provider.h"
#include "prompt.h"
#include "oidc_utilities.h"
#include "json.h"
#include "file_io.h"
#include "crypt.h"
#include "ipc.h"


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
    case 'r':
      arguments->registering = 1;
      break;
    case 'o':
      arguments->output = arg;
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
  arguments.registering = 0;
  arguments.verbose = 0;
  arguments.output = NULL;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  if(arguments.registering) {
    struct connection con = {0,0,0};
    if(ipc_init(&con, OIDC_SOCK_ENV_NAME, 0)!=0) {
      exit(EXIT_FAILURE);
    }
    if(ipc_connect(con)<0) {
      printf("Could not connect to oicd\n");
      exit(EXIT_FAILURE);
    }
    registerClient(*(con.sock), arguments.args[0], arguments);
    exit(EXIT_SUCCESS);
  }

  if(arguments.delete) {
    handleDelete(arguments.args[0]);
    exit(EXIT_SUCCESS);
  } 

  if(arguments.file) {
    char* inputconfig = readFile(arguments.file);
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Read config from user provided file: %s", inputconfig);
    if(!inputconfig) {
      fprintf(stderr, "Could not read config file: %s\n", oidc_perror());
      exit(EXIT_FAILURE);
    }
    provider = getProviderFromJSON(inputconfig);
    clearFreeString(inputconfig);
  }
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
    printf("An unexpected error occured. It's seems that oidc-agent has stopped.\n That's not good.\n");
    exit(EXIT_FAILURE);
  }

  struct key_value pairs[7];
  pairs[0].key = "status";
  pairs[1].key = "refresh_token";
  pairs[2].key = "token_endpoint";
  pairs[3].key = "authorization_endpoint";
  pairs[4].key = "registration_endpoint";
  pairs[5].key = "revocation_endpoint";
  pairs[6].key = "error";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printf("Could not decode json: %s\n", res);
    printf("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    saveExit(EXIT_FAILURE);
  }
  clearFreeString(res);
  if(pairs[6].value!=NULL) {
    printf("Error: %s\n", pairs[6].value);
    clearFreeString(pairs[6].value);clearFreeString(pairs[5].value); clearFreeString(pairs[4].value); clearFreeString(pairs[3].value);  clearFreeString(pairs[2].value); clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
    saveExit(EXIT_FAILURE);
  }
  if(pairs[2].value!=NULL) {
    provider_setTokenEndpoint(provider, pairs[2].value);
  } else {
    fprintf(stderr, "Error: response does not contain token_endpoint\n");
  }
  if(pairs[3].value!=NULL) {
    provider_setAuthorizationEndpoint(provider, pairs[3].value);
  } else {
    fprintf(stderr, "Error: response does not contain authorization_endpoint\n");
  }
  if(pairs[4].value!=NULL) {
    provider_setRegistrationEndpoint(provider, pairs[4].value);
  } else {
    fprintf(stderr, "Error: response does not contain registration_endpoint\n");
  }
  if(pairs[5].value!=NULL) {
    provider_setRevocationEndpoint(provider, pairs[5].value);
  } else {
    fprintf(stderr, "Error: response does not contain revocation_endpoint\n");
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
  encryptAndWriteConfig(json, encryptionPassword, NULL, provider->name);
  clearFreeString(json);
  saveExit(EXIT_SUCCESS);
  return EXIT_FAILURE;
}

oidc_error_t encryptAndWriteConfig(const char* text, const char* suggestedPassword, const char* filepath, const char* oidc_filename) {
  initCrypt();
  char* encryptionPassword = getEncryptionPassword(suggestedPassword, UINT_MAX); 
  if(encryptionPassword==NULL) {
    return oidc_errno;
  }
  char* toWrite = encryptProvider(text, encryptionPassword);
  clearFreeString(encryptionPassword);
  if(toWrite==NULL) {
    return oidc_errno;
  }
  if(filepath) {
    writeFile(filepath, toWrite);
  } else {
    writeOidcFile(oidc_filename, toWrite);
  }
  clearFreeString(toWrite);
  return OIDC_SUCCESS;
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
  if(provider==NULL) {
    provider = calloc(sizeof(struct oidc_provider), 1);
  }
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
      if(getUserConfirmation("A provider with this short name is already configured. Do you want to edit the configuration?")) {
        struct oidc_provider* loaded_p = NULL;
        while(NULL==loaded_p) {
          encryptionPassword = promptPassword("Enter encryption Password: ");
          loaded_p = decryptProvider(provider_getName(*provider), encryptionPassword);
        }
        freeProvider(provider);
        provider = loaded_p;
        break;
      } else {
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

char* getEncryptionPassword(const char* suggestedPassword, unsigned int max_pass_tries) {
  char* encryptionPassword = NULL;
  unsigned int i;
  unsigned int max_tries = max_pass_tries==0 ? MAX_PASS_TRIES : max_pass_tries;
  for(i=0; i<max_tries; i++) {
    char* input = promptPassword("Enter encrpytion password%s: ", suggestedPassword ? " [***]" : "");
    if(suggestedPassword && !isValid(input)) { // use same encrpytion password
      clearFreeString(input);
      encryptionPassword = calloc(sizeof(char), strlen(suggestedPassword)+1);
      strcpy(encryptionPassword, suggestedPassword);
      return encryptionPassword;
    } else {
      encryptionPassword = input;
      char* confirm = promptPassword("Confirm encryption Password: ");
      if(strcmp(encryptionPassword, confirm)!=0) {
        printf("Encryption passwords did not match.\n");
        clearFreeString(confirm);
      } else {
        clearFreeString(confirm);
        break;
      }
    } 
  } 
  if(encryptionPassword) {
    clearFreeString(encryptionPassword);
  }

  oidc_errno = OIDC_EMAXTRIES;
  return NULL;
}

void registerClient(int sock, char* short_name, struct arguments arguments) {
  provider = calloc(sizeof(struct oidc_provider), 1);
  if(short_name) { 
    char* name = calloc(sizeof(char), strlen(short_name)+1);
    strcpy(name, short_name);
    provider_setName(provider, name); 
  }
  while(!isValid(provider_getName(*provider))) {
    provider_setName(provider, prompt("Enter short name for the provider to configure: ")); 
    if(!isValid(provider_getName(*provider))) {
      continue;
    }
  }
  if(oidcFileDoesExist(provider_getName(*provider))) {
    fprintf(stderr, "A provider with that shortname already configured\n");
    exit(EXIT_FAILURE);
  } 

  unsigned int i;
  for(i=0; i<sizeof(possibleCertFiles)/sizeof(*possibleCertFiles); i++) {
    if(fileDoesExist(possibleCertFiles[i])) {
      char* cert_path = calloc(sizeof(char), strlen(possibleCertFiles[i])+1);
      strcpy(cert_path, possibleCertFiles[i]);
      provider_setCertPath(provider, cert_path);
      break;
    }
  }
  promptAndSet("Cert Path%s%s%s: ", provider_setCertPath, provider_getCertPath, 0, 0);
  promptAndSetIssuer();

  char* json = providerToJSON(*provider);

  ipc_write(sock, REQUEST_CONFIG, "register", json);
  clearFreeString(json);
  char* res = ipc_read(sock);
  if(arguments.verbose) {
    printf("%s\n", res);
  }
  struct key_value pairs[4];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "client";
  pairs[3].key = "info";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printf("Could not decode json: %s\n", res);
    printf("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    saveExit(EXIT_FAILURE);
  }
  if(pairs[1].value) {
    printf("Error: %s\n", pairs[1].value);
  }
  if(pairs[2].value) {
    char* client_config = pairs[2].value;
    if(arguments.output) {
      printf("Writing the following client config to file '%s'\n%s\n", arguments.output, client_config);
      encryptAndWriteConfig(client_config, NULL, arguments.output, NULL);
    } else {
      char* name = getJSONValue(client_config, "client_name");
      name = realloc(name, strlen(name)+strlen(".clientconfig")+1);
      strcat(name, ".clientconfig");
      if(oidcFileDoesExist(name)) {
        syslog(LOG_AUTHPRIV|LOG_DEBUG, "The clientconfig file already exists. Changing path.");
        int i = 0;
        char* newName = NULL;
        do {
          clearFreeString(newName);
          newName = calloc(sizeof(char), snprintf(NULL, 0, "%s%d", name, i));
          sprintf(newName, "%s%d", name, i);
          i++;
        } while(oidcFileDoesExist(newName));
        clearFreeString(name);
        name = newName;
      }
      printf("Writing the following client config to file '%s/%s'\n%s\n", getOidcDir(), name, client_config);
      encryptAndWriteConfig(client_config, NULL, NULL, name);
      clearFreeString(name);
    }

  }
  if(pairs[3].value) {
    printf("%s\n", pairs[3].value);
    clearFreeString(pairs[3].value);
  }
  clearFreeString(pairs[0].value);
  clearFreeString(pairs[1].value);
  clearFreeString(pairs[2].value);
  clearFreeString(res);
}

void handleDelete(char* short_name) {
  if(!oidcFileDoesExist(short_name)) {
    fprintf(stderr, "No provider with that shortname configured\n");
    exit(EXIT_FAILURE);
  } 
  struct oidc_provider* loaded_p = NULL;
  while(NULL==loaded_p) {
    encryptionPassword = promptPassword("Enter encryption Password: ");
    loaded_p = decryptProvider(short_name, encryptionPassword);
  }
  char* json = providerToJSON(*loaded_p);
  freeProvider(loaded_p);
  deleteClient(short_name, json, 1);
  clearFreeString(json);
}

void deleteClient(char* short_name, char* provider_json, int revoke) {


  struct connection con = {0,0,0};
  if(ipc_init(&con, OIDC_SOCK_ENV_NAME, 0)!=0) {
    exit(EXIT_FAILURE);
  }
  if(ipc_connect(con)<0) {
    printf("Could not connect to oicd\n");
    exit(EXIT_FAILURE);
  }
  ipc_write(*(con.sock), REQUEST_CONFIG, revoke ? "delete" : "remove", provider_json);
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
    if(removeOidcFile(short_name)==0) {
      printf("Successfully deleted provider configuration.\n");
    } else {
      printf("error removing configuration file: %s", oidc_perror());
    }

    exit(EXIT_SUCCESS);
  }
  if(pairs[1].value!=NULL) {
    printf("Error: %s\n", pairs[1].value);
    if(strstarts(pairs[1].value, "Could not revoke token:")) {
      if(getUserConfirmation("Do you want to unload and delete anyway. You then have to revoke the refresh token manually.")) {
        deleteClient(short_name, provider_json, 0);
      } else {
        printf("The provider was not removed from oidc-agent due to the above listed error. You can fix the error and try it again.\n");
      }
    } else {
      printf("The provider was not removed from oidc-agent due to the above listed error. You can fix the error and try it again.\n");
    }
    clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
    exit(EXIT_FAILURE);
  }

}
