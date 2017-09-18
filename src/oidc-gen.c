#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "oidc-gen.h"
#include "account.h"
#include "prompt.h"
#include "oidc_utilities.h"
#include "json.h"
#include "file_io.h"
#include "crypt.h"
#include "api.h"
#include "ipc.h"
#include "settings.h"


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
      arguments->manual = 1;
      break;
    case 'm':
      arguments->manual = 1;
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

static char args_doc[] = "[SHORT_NAME]";

static char doc[] = "oidc-gen -- A tool for generating oidc account configurations which can be used by oidc-add";

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void initArguments(struct arguments* arguments) {
  arguments->delete = 0;
  arguments->debug = 0;
  arguments->args[0] = NULL;
  arguments->file = NULL;
  arguments->manual = 0;
  arguments->verbose = 0;
  arguments->output = NULL;
}

void assertOidcDirExists() {
  char* dir = NULL;
  if((dir = getOidcDir())==NULL) {
    printf("Error: oidc-dir does not exist. Run make to create it.\n");
    exit(EXIT_FAILURE);
  }
  clearFreeString(dir);
}

struct oidc_account* accountFromFile(const char* filename) {
  char* inputconfig = readFile(filename);
  if(!inputconfig) {
    fprintf(stderr, "Could not read config file: %s\n", oidc_perror());
    exit(EXIT_FAILURE);
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Read config from user provided file: %s", inputconfig);
  struct oidc_account* account = getAccountFromJSON(inputconfig);
  if(!account) {
    char* encryptionPassword = NULL;
    int i;
    for(i=0; i<MAX_PASS_TRIES && account==NULL; i++) {
      clearFreeString(encryptionPassword);
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "Read config from user provided file: %s", inputconfig);
      encryptionPassword = promptPassword("Enter decryption Password: ");
      account = decryptAccountText(inputconfig, encryptionPassword);
    }
    if(account!=NULL) {
      account_setRefreshToken(account, NULL); // currently not read correctly, there won't be any valid one in it
    }
  }
  clearFreeString(inputconfig);
  return account;
}

int main(int argc, char** argv) {
  openlog("oidc-gen", LOG_CONS|LOG_PID, LOG_AUTHPRIV);
  setlogmask(LOG_UPTO(LOG_NOTICE));

  struct arguments arguments;
  initArguments(&arguments);
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  if(arguments.debug) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  assertOidcDirExists();

  if(arguments.delete) {
    handleDelete(arguments.args[0]);
    exit(EXIT_SUCCESS);
  } 

  struct oidc_account* account = NULL;
  if(arguments.file) {
    account = accountFromFile(arguments.file); 
  }
  if(arguments.manual) {
    manualGen(account, arguments.args[0], arguments.verbose);
  } else {
    registerClient(arguments.args[0], arguments.output, arguments.verbose);
  }
  exit(EXIT_SUCCESS);

}

// if issuer isn't already in issuer.config than add it
void updateIssuerConfig(const char* issuer_url) {
  char* issuers = readOidcFile(ISSUER_CONFIG_FILENAME);
  if(strcasestr(issuers, issuer_url)!=NULL) {
    clearFreeString(issuers);
    return;
  }
  char* new_issuers = oidc_sprintf("%s%s", issuers, issuer_url);
  clearFreeString(issuers);
  writeOidcFile(ISSUER_CONFIG_FILENAME, new_issuers);
  clearFreeString(new_issuers);
}

void manualGen(struct oidc_account* account, const char* short_name, int verbose) {
  account = genNewAccount(account, short_name);
  char* json = accountToJSON(*account);

  char* res = communicate(REQUEST_CONFIG, "gen", json);

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
    account_setTokenEndpoint(account, pairs[2].value);
  } else {
    fprintf(stderr, "Error: response does not contain token_endpoint\n");
  }
  if(pairs[3].value!=NULL) {
    account_setAuthorizationEndpoint(account, pairs[3].value);
  } else {
    fprintf(stderr, "Error: response does not contain authorization_endpoint\n");
  }
  if(pairs[4].value!=NULL) {
    account_setRegistrationEndpoint(account, pairs[4].value);
  } else {
    fprintf(stderr, "Error: response does not contain registration_endpoint\n");
  }
  if(pairs[5].value!=NULL) {
    account_setRevocationEndpoint(account, pairs[5].value);
  } else {
    fprintf(stderr, "Error: response does not contain revocation_endpoint\n");
  }
  if(pairs[1].value!=NULL) {
    account_setRefreshToken(account, pairs[1].value);
  }
  printf("%s\n", pairs[0].value);
  if(strcmp(pairs[0].value, "success")==0) {
    printf("The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.\n");
  }
  clearFreeString(pairs[0].value);

  // remove username and password from config
  account_setUsername(account, NULL);
  account_setPassword(account, NULL);
  clearFreeString(json);
  json = accountToJSON(*account);

  updateIssuerConfig(account_getIssuer(*account));

  if(verbose) {
    printf("The following data will be saved encrypted:\n%s\n", json);
  }

  //encrypt
  encryptAndWriteConfig(json, NULL, NULL, account->name);
  clearFreeString(json);
  saveExit(EXIT_SUCCESS);
}

oidc_error_t encryptAndWriteConfig(const char* text, const char* suggestedPassword, const char* filepath, const char* oidc_filename) {
  initCrypt();
  char* encryptionPassword = getEncryptionPassword(suggestedPassword, UINT_MAX); 
  if(encryptionPassword==NULL) {
    return oidc_errno;
  }
  char* toWrite = encryptAccount(text, encryptionPassword);
  clearFreeString(encryptionPassword);
  if(toWrite==NULL) {
    return oidc_errno;
  }
  if(filepath) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Write to file %s", filepath);
    writeFile(filepath, toWrite);
  } else {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "Write to oidc file %s", oidc_filename);
    writeOidcFile(oidc_filename, toWrite);
  }
  clearFreeString(toWrite);
  return OIDC_SUCCESS;
}

void promptAndSet(struct oidc_account* account, char* prompt_str, void (*set_callback)(struct oidc_account*, char*), char* (*get_callback)(struct oidc_account), int passPrompt, int optional) {
  char* input = NULL;
  do {
    if(passPrompt) {
      input = promptPassword(prompt_str, isValid(get_callback(*account)) ? " [***]" : "");
    } else {
      input = prompt(prompt_str, isValid(get_callback(*account)) ? " [" : "", isValid(get_callback(*account)) ? get_callback(*account) : "", isValid(get_callback(*account)) ? "]" : "");
    }
    if(isValid(input)) {
      set_callback(account, input);
    } else {
      clearFreeString(input);
    }
    if(optional) {
      break;
    }
  } while(!isValid(get_callback(*account)));
}

int promptIssuer(struct oidc_account* account, const char* fav) {
  char* input = prompt("Issuer [%s]: ", fav);
  if(!isValid(input)) {
    char* iss = calloc(sizeof(char), strlen(fav)+1);
    strcpy(iss, fav);
    clearFreeString(input);
    account_setIssuer(account, iss);
    return -1;
  } else if (isdigit(*input)) {
    int i = atoi(input);
    clearFreeString(input);
    i--; // printed indices starts at 1 for non nerds
    return i;
  } else {
    account_setIssuer(account, input);
    return -1;
  }
}

void useSuggestedIssuer(struct oidc_account* account) {
  char* fileContent = readOidcFile(ISSUER_CONFIG_FILENAME);
  char* s = fileContent;
  int i;
  for (i=0; s[i]; s[i]=='\n' ? i++ : *s++); // counts the lines in the file
  int size = i++;
  char* accounts[size];
  accounts[0] = strtok(fileContent, "\n");
  for(i=1; i<size; i++) {
    accounts[i] = strtok(NULL, "\n");
    if(accounts[i]==NULL) {
      size=i;
    }
  }
  char* fav = accounts[0];
  for(i=size-1; i>=0; i--) {
    if(strcasestr(accounts[i], account_getName(*account))) { // if the short name is a substring of the issuer it's likely that this is the fav issuer
      fav = accounts[i];
    }
  }
  if(isValid(account_getIssuer(*account))) {
    fav = account_getIssuer(*account);
  }
  for(i=0; i<size; i++) {
    printf("[%d] %s\n", i+1, accounts[i]); // printed indices starts at 1 for non nerd
  }
  clearFreeString(fileContent);

  while((i = promptIssuer(account, fav)) >= size) {
    printf("input out of bound\n");
  } 
  if(i>=0) {
    char* iss = calloc(sizeof(char), strlen(accounts[i])+1);
    strcpy(iss, accounts[i]);
    account_setIssuer(account, iss);
  }
}

void stringifyIssuerUrl(struct oidc_account* account) {
  int issuer_len = strlen(account_getIssuer(*account));
  if(account_getIssuer(*account)[issuer_len-1]!='/') {
    void* tmp = realloc(account_getIssuer(*account), issuer_len+1+1);   
    if(NULL==tmp) {
      printf("realloc failed\n");
      exit(EXIT_FAILURE);
    }
    account->issuer = tmp; // don't use account_setIssuer here, because the free in it would double free because of realloc
    account_getIssuer(*account)[issuer_len] = '/';
    issuer_len = strlen(account_getIssuer(*account));
    account_getIssuer(*account)[issuer_len] = '\0';
  }

}

void promptAndSetIssuer(struct oidc_account* account) {
  if(!oidcFileDoesExist(ISSUER_CONFIG_FILENAME)) {
    promptAndSet(account, "Issuer%s%s%s: ", account_setIssuer, account_getIssuer, 0, 0);
  } else {
    useSuggestedIssuer(account); 
  }
  stringifyIssuerUrl(account);

  //TODO remove
  account_setConfigEndpoint(account, calloc(sizeof(char), issuer_len + strlen(CONF_ENDPOINT_SUFFIX) + 1));
  sprintf(account_getConfigEndpoint(*account), "%s%s", account_getIssuer(*account), CONF_ENDPOINT_SUFFIX);
}

void promptAndSetClientId(struct oidc_account* account) {
  promptAndSet(account, "Client_id%s%s%s: ", account_setClientId, account_getClientId, 0, 0);
}

void promptAndSetClientSecret(struct oidc_account* account) {
  promptAndSet(account, "Client_secret%s%s%s: ", account_setClientSecret, account_getClientSecret, 0, 0);
}

void promptAndSetRefreshToken(struct oidc_account* account) {
  promptAndSet(account, "Refresh token%s%s%s: ", account_setRefreshToken, account_getRefreshToken, 0, 1);
}

void promptAndSetUsername(struct oidc_account* account) {
  promptAndSet(account, "Username%s%s%s: ", account_setUsername, account_getUsername, 0, isValid(account_getRefreshToken(*account)));
}

void promptAndSetPassword(struct oidc_account* account) {
  promptAndSet(account, "Password%s: ", account_setPassword, account_getPassword, 1, isValid(account_getRefreshToken(*account)));
}

void promptAndSetCertPath(struct oidc_account* account) {
  if(!isValid(account_getCertPath(*account))) {
    unsigned int i;
    for(i=0; i<sizeof(possibleCertFiles)/sizeof(*possibleCertFiles); i++) {
      if(fileDoesExist(possibleCertFiles[i])) {
        char* cert_path = calloc(sizeof(char), strlen(possibleCertFiles[i])+1);
        strcpy(cert_path, possibleCertFiles[i]);
        account_setCertPath(account, cert_path);
        break;
      }
    }
  }
  promptAndSet(account, "Cert Path%s%s%s: ", account_setCertPath, account_getCertPath, 0, 0);
}

void promptAndSetName(struct oidc_account* account, const char* short_name) {
  if(short_name) {
    char* name = calloc(sizeof(char), strlen(short_name)+1);
    strcpy(name, short_name);
    account_setName(account, name);
  } else {
    while(!isValid(account_getName(*account))) {
      account_setName(account, prompt("Enter short name for the account to configure: ")); 
    }
  }

}

struct oidc_account* genNewAccount(struct oidc_account* account, const char* short_name) {
  if(account==NULL) {
    account = calloc(sizeof(struct oidc_account), 1);
  }
  promptAndSetName(account, short_name);
  char* encryptionPassword = NULL;
  if(oidcFileDoesExist(account_getName(*account))) {
    struct oidc_account* loaded_p = NULL;
    unsigned int i;
    for(i=0; i<MAX_PASS_TRIES && NULL==loaded_p; i++) {
      clearFreeString(encryptionPassword);
      encryptionPassword = promptPassword("Enter encryption Password: ");
      loaded_p = decryptAccount(account_getName(*account), encryptionPassword);
    }
    freeAccount(account);
    account = loaded_p;
  } else {
    printf("No account exists with this short name. Creating new configuration ...\n");
  }
  promptAndSetCertPath(account);
  promptAndSetIssuer(account);
  promptAndSetClientId(account);
  promptAndSetClientSecret(account);
  promptAndSetRefreshToken(account);
  promptAndSetUsername(account);
  promptAndSetPassword(account);

  return account;
}

char* encryptAccount(const char* json, const char* password) {
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
    char* input = promptPassword("Enter encrpytion password%s: ", isValid(suggestedPassword) ? " [***]" : "");
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
        return encryptionPassword;
      }
    } 
  } 
  if(encryptionPassword) {
    clearFreeString(encryptionPassword);
  }

  oidc_errno = OIDC_EMAXTRIES;
  return NULL;
}

char* createClientConfigFileName(const char* issuer_url, const char* client_id) {
  char* path_fmt = "%s_%s_%s.clientconfig";
  char* iss = calloc(sizeof(char), strlen(issuer_url+8)+1); // +8 cuts 'https://' 
  strcpy(iss, issuer_url+8); // +8 cuts 'https://' 
  char* iss_new_end = strchr(iss, '/'); // cut after the first '/'
  *iss_new_end = 0;
  char* today = getDateString();
  char* path = calloc(sizeof(char), snprintf(NULL, 0, path_fmt, iss, today, client_id)+1);
  sprintf(path, path_fmt, iss, today, client_id);
  clearFreeString(today);


  if(oidcFileDoesExist(path)) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "The clientconfig file already exists. Changing path.");
    int i = 0;
    char* newName = NULL;
    do {
      clearFreeString(newName);
      newName = calloc(sizeof(char), snprintf(NULL, 0, "%s%d", path, i));
      sprintf(newName, "%s%d", path, i);
      i++;
    } while(oidcFileDoesExist(newName));
    clearFreeString(path);
    path = newName;
  }
  return path;
}

void registerClient(char* short_name, const char* output, int verbose) {
  struct oidc_account* account = calloc(sizeof(struct oidc_account), 1);
  promptAndSetName(account, short_name);
  if(oidcFileDoesExist(account_getName(*account))) {
    fprintf(stderr, "A account with that shortname already configured\n");
    exit(EXIT_FAILURE);
  } 
  promptAndSetCertPath(account);
  promptAndSetIssuer(account);

  char* json = accountToJSON(*account);

  char* res = communicate(REQUEST_CONFIG, "register", json);
  clearFreeString(json);
  if(verbose) {
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
  if(pairs[3].value) {
    printf("%s\n", pairs[3].value);
    clearFreeString(pairs[3].value);
  }
  if(pairs[2].value) {
    char* client_config = pairs[2].value;
    if(output) {
      printf("Writing client config to file '%s'\n", output);
      encryptAndWriteConfig(client_config, NULL, output, NULL);
    } else {
      char* client_id = getJSONValue(client_config, "client_id");
      char* path = createClientConfigFileName(account_getIssuer(*account), client_id);
      clearFreeString(client_id);
      printf("Writing client config to file '%s%s'\n", getOidcDir(), path);
      encryptAndWriteConfig(client_config, NULL, NULL, path);
      clearFreeString(path);
    }

  }
  clearFreeString(pairs[0].value);
  clearFreeString(pairs[1].value);
  clearFreeString(pairs[2].value);
  clearFreeString(res);
}

void handleDelete(char* short_name) {
  if(!oidcFileDoesExist(short_name)) {
    fprintf(stderr, "No account with that shortname configured\n");
    exit(EXIT_FAILURE);
  } 
  struct oidc_account* loaded_p = NULL;
  char* encryptionPassword = NULL;
  unsigned int i;
  for(i=0; i<MAX_PASS_TRIES && NULL==loaded_p; i++) {
    encryptionPassword = getEncryptionPassword(NULL, MAX_PASS_TRIES-i);
    loaded_p = decryptAccount(short_name, encryptionPassword);
    clearFreeString(encryptionPassword);
  }
  char* json = accountToJSON(*loaded_p);
  freeAccount(loaded_p);
  deleteClient(short_name, json, 1);
  clearFreeString(json);
}

void deleteClient(char* short_name, char* account_json, int revoke) {
  char* res = communicate(REQUEST_CONFIG, revoke ? "delete" : "remove", account_json);

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
  if(strcmp(pairs[0].value, "success")==0 || strcmp(pairs[1].value, "account not loaded")==0) {
    printf("The generated account was successfully removed from oidc-agent. You don't have to run oidc-add.\n");
    clearFreeString(pairs[0].value);
    if(removeOidcFile(short_name)==0) {
      printf("Successfully deleted account configuration.\n");
    } else {
      printf("error removing configuration file: %s", oidc_perror());
    }

    exit(EXIT_SUCCESS);
  }
  if(pairs[1].value!=NULL) {
    printf("Error: %s\n", pairs[1].value);
    if(strstarts(pairs[1].value, "Could not revoke token:")) {
      if(getUserConfirmation("Do you want to unload and delete anyway. You then have to revoke the refresh token manually.")) {
        deleteClient(short_name, account_json, 0);
      } else {
        printf("The account was not removed from oidc-agent due to the above listed error. You can fix the error and try it again.\n");
      }
    } else {
      printf("The account was not removed from oidc-agent due to the above listed error. You can fix the error and try it again.\n");
    }
    clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
    exit(EXIT_FAILURE);
  }

}
