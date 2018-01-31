#define _GNU_SOURCE

#include "gen_handler.h"
#include "api.h"
#include "crypt.h"
#include "prompt.h"
#include "file_io.h"
#include "settings.h"
#include "parse_ipc.h"
#include "oidc_utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <syslog.h>

void handleGen(struct oidc_account* account, int verbose, char* flow, char** cryptPassPtr) {
  char* json = accountToJSON(*account);
  freeAccount(account);
  if(flow==NULL) {
    flow = json_hasKey(json, "redirect_uris") ? FLOW_VALUE_CODE : FLOW_VALUE_PASSWORD;
  }
  if(strchr(flow, ' ')!=NULL) {
    flow = delimitedListToJSONArray(flow, ' ');
  } else {
    flow = oidc_sprintf("\"%s\"", flow);
  }
  printf("Generating account configuration ...");
  char* res = communicate(REQUEST_CONFIG_FLOW, REQUEST_VALUE_GEN, json, flow);
  clearFreeString(flow);
  clearFreeString(json); json = NULL;
  if(NULL==res) {
    printError("Error: %s\n", oidc_serror());
    if(cryptPassPtr) {
      clearFreeString(*cryptPassPtr);
      clearFree(cryptPassPtr, sizeof(char*));
    }
    exit(EXIT_FAILURE);
  }
  json = gen_parseResponse(res, verbose);

  char* issuer = getJSONValue(json, "issuer_url");
  updateIssuerConfig(issuer);
  clearFreeString(issuer);

  if(verbose) {
    printf("The following data will be saved encrypted:\n%s\n", json);
  }
  char* name = getJSONValue(json, "name");
  char* hint = oidc_sprintf("account configuration '%s'", name);
  encryptAndWriteConfig(json, hint, cryptPassPtr ? *cryptPassPtr : NULL, NULL, name);
  clearFreeString(name);
  clearFreeString(hint);

  if(cryptPassPtr) {
    clearFreeString(*cryptPassPtr);
    clearFree(cryptPassPtr, sizeof(char*));
  }
  clearFreeString(json);
}

void manualGen(struct oidc_account* account, const char* short_name, int verbose, char* flow, struct optional_arg cert_path) {
  char** cryptPassPtr = calloc(sizeof(char*), 1);
  account = genNewAccount(account, short_name, cryptPassPtr, cert_path);
  handleGen(account, verbose, flow, cryptPassPtr); 
}

void handleCodeExchange(char* request, char* short_name, int verbose) {
  int needFree = 0;
  while(!isValid(short_name)) {
    if(needFree) {
      clearFreeString(short_name);
    }
    short_name = prompt("Enter short name for the account to configure: "); 
    needFree = 1;
  }

  char* res = communicate(request);
  if(NULL==res) {
    printError("Error: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
    if(needFree) {
      clearFreeString(short_name);
    }
  }
  char* config = gen_parseResponse(res, verbose);
  if(verbose) {
    printf("The following data will be saved encrypted:\n%s\n", config);
  }

  char* hint = oidc_sprintf("account configuration '%s'", short_name);
  encryptAndWriteConfig(config, hint, NULL, NULL, short_name);
  clearFreeString(hint);
  if(needFree) {
    clearFreeString(short_name);
  }
  clearFreeString(config);
}

void handleStateLookUp(char* state, int verbose) {
  char* res = NULL;
  char* config = NULL;
  printf("Polling oidc-agent to get the generated account configuration ...");
  fflush(stdout);
  int i = 0;
  while(config==NULL && i<MAX_POLL) { 
    i++;
    res = communicate(REQUEST_STATELOOKUP, state);
    if(NULL==res) {
      printf("\n");
      printError("Error: %s\n", oidc_serror());
      exit(EXIT_FAILURE);
    }
    config = gen_parseResponse(res, verbose);
    if(config==NULL) {
      usleep(DELTA_POLL*1000);
      printf(".");
      fflush(stdout);
    }
  }
  printf("\n");
  if(i==MAX_POLL) {
    printf("Polling is boring. Already tried %d times. I stop now.\n" C_IMPORTANT "Please press Enter to try it again.\n" C_RESET, i);
    getchar();
    res = communicate(REQUEST_STATELOOKUP, state);
    if(res==NULL) {
      printError("Error: %s\n", oidc_serror());
      exit(EXIT_FAILURE);
    }
    config = gen_parseResponse(res, verbose);
    if(config==NULL) {
      printError("Could not receive generated account configuration for state='%s'\n" C_IMPORTANT "Please try state lookup again by using:\noidc-gen --state=%s\n", state, state);
      exit(EXIT_FAILURE);
    }
  }
  char* issuer = getJSONValue(config, "issuer_url");
  updateIssuerConfig(issuer);
  clearFreeString(issuer);

  if(verbose) {
    printf("The following data will be saved encrypted:\n%s\n", config);
  }

  char* short_name = getJSONValue(config, "name");
  char* hint = oidc_sprintf("account configuration '%s'", short_name);
  encryptAndWriteConfig(config, hint, NULL, NULL, short_name);
  clearFreeString(hint);
  clearFreeString(short_name);
  clearFreeString(config);
  exit(EXIT_SUCCESS);
}

struct oidc_account* genNewAccount(struct oidc_account* account, const char* short_name, char** cryptPassPtr, struct optional_arg cert_path) {
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
      char* prompt = oidc_sprintf("Enter encryption Password for account config '%s': ", account_getName(*account));
      encryptionPassword = promptPassword(prompt);
      clearFreeString(prompt);
      loaded_p = decryptAccount(account_getName(*account), encryptionPassword);
    }
    freeAccount(account);
    account = loaded_p;
  } else {
    printf("No account exists with this short name. Creating new configuration ...\n");
  }
  promptAndSetCertPath(account, cert_path);
  promptAndSetIssuer(account);
  promptAndSetClientId(account);
  promptAndSetClientSecret(account);
  promptAndSetScope(account);
  promptAndSetRefreshToken(account);
  promptAndSetUsername(account);
  promptAndSetPassword(account);
  promptAndSetRedirectUris(account);
  *cryptPassPtr = encryptionPassword;
  return account;
}

struct oidc_account* registerClient(char* short_name, const char* output, int verbose, struct optional_arg token, struct optional_arg cert_path) {
  struct oidc_account* account = calloc(sizeof(struct oidc_account), 1);
  promptAndSetName(account, short_name);
  if(oidcFileDoesExist(account_getName(*account))) {
    printError("A account with that shortname already configured\n");
    exit(EXIT_FAILURE);
  } 

  promptAndSetCertPath(account, cert_path);
  promptAndSetIssuer(account);
  promptAndSetScope(account);
  char* authorization = NULL;
  if(token.useIt) {
    if(token.str) {
      authorization = token.str;
    } else {
    authorization = prompt("Registration endpoint authorization access token: ");
    }
  }

  char* json = accountToJSON(*account);
  printf("Registering Client ...\n");
  char* res = communicate(REQUEST_CONFIG_AUTH, REQUEST_VALUE_REGISTER, json, authorization?:"");
  if(token.useIt && token.str == NULL) {
  clearFreeString(authorization);
  }
  clearFreeString(json);
  if(NULL==res) {
    printError("Error: %s\n", oidc_serror());
    freeAccount(account);
    exit(EXIT_FAILURE);
  }
  if(verbose && res) {
    printf("%s\n", res);
  }
  struct key_value pairs[4];
  pairs[0].key = "status";
  pairs[1].key = "error";
  pairs[2].key = "client";
  pairs[3].key = "info";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    exit(EXIT_FAILURE);
  }
  clearFreeString(res);
  if(pairs[1].value) {
    printError("Error: %s\n", pairs[1].value);
  }
  if(pairs[3].value) {
    printf(C_IMPORTANT "%s\n" C_RESET, pairs[3].value);
    clearFreeString(pairs[3].value);
  }
  clearFreeString(pairs[0].value);
  clearFreeString(pairs[1].value);
  if(pairs[2].value) {
    char* client_config = pairs[2].value;
    if(output) {
      printf(C_IMPORTANT "Writing client config to file '%s'\n" C_RESET, output);
      encryptAndWriteConfig(client_config,  "client config file", NULL, output, NULL);
    } else {
      char* client_id = getJSONValue(client_config, "client_id");
      char* path = createClientConfigFileName(account_getIssuerUrl(*account), client_id);
      clearFreeString(client_id);
      char* oidcdir = getOidcDir();
      printf(C_IMPORTANT "Writing client config to file '%s%s'\n" C_RESET, oidcdir, path);
      clearFreeString(oidcdir);
      encryptAndWriteConfig(client_config, "client config file", NULL, NULL, path);
      clearFreeString(path);
    }
    struct oidc_account* updatedAccount = getAccountFromJSON(client_config);
    clearFreeString(client_config);
    account_setIssuerUrl(updatedAccount, oidc_strcopy(account_getIssuerUrl(*account)));
    account_setName(updatedAccount, oidc_strcopy(account_getName(*account)));
    account_setCertPath(updatedAccount, oidc_strcopy(account_getCertPath(*account)));
    freeAccount(account);
    return updatedAccount;
  }
  clearFreeString(pairs[2].value);
  freeAccount(account);
  return NULL;
}

void handleDelete(char* short_name) {
  if(!oidcFileDoesExist(short_name)) {
    printError("No account with that shortname configured\n");
    exit(EXIT_FAILURE);
  } 
  struct oidc_account* loaded_p = NULL;
  char* encryptionPassword = NULL;
  unsigned int i;
  for(i=0; i<MAX_PASS_TRIES && NULL==loaded_p; i++) {
    char* forWhat = oidc_sprintf("account config '%s'", short_name);
    encryptionPassword = getEncryptionPassword(forWhat, NULL, MAX_PASS_TRIES-i);
    clearFreeString(forWhat);
    loaded_p = decryptAccount(short_name, encryptionPassword);
    clearFreeString(encryptionPassword);
  }
  char* json = accountToJSON(*loaded_p);
  freeAccount(loaded_p);
  deleteClient(short_name, json, 1);
  clearFreeString(json);
}

void deleteClient(char* short_name, char* account_json, int revoke) {
  char* res = communicate(REQUEST_CONFIG, revoke ? REQUEST_VALUE_DELETE : REQUEST_VALUE_REMOVE, account_json);

  struct key_value pairs[2];
  pairs[0].key = "status";
  pairs[1].key = "error";
  if(getJSONValues(res, pairs, sizeof(pairs)/sizeof(*pairs))<0) {
    printError("Could not decode json: %s\n", res);
    printError("This seems to be a bug. Please hand in a bug report.\n");
    clearFreeString(res);
    exit(EXIT_FAILURE);
  }
  clearFreeString(res);
  if(strcmp(pairs[0].value, STATUS_SUCCESS)==0 || strcmp(pairs[1].value, ACCOUNT_NOT_LOADED)==0) {
    printf("The generated account was successfully removed from oidc-agent. You don't have to run oidc-add.\n");
    clearFreeString(pairs[0].value);
    if(removeOidcFile(short_name)==0) {
      printf("Successfully deleted account configuration.\n");
    } else {
      printError("error removing configuration file: %s", oidc_serror());
    }

    exit(EXIT_SUCCESS);
  }
  if(pairs[1].value!=NULL) {
    printError("Error: %s\n", pairs[1].value);
    if(strstarts(pairs[1].value, "Could not revoke token:")) {
      if(getUserConfirmation("Do you want to unload and delete anyway. You then have to revoke the refresh token manually.")) {
        deleteClient(short_name, account_json, 0);
      } else {
        printError("The account was not removed from oidc-agent due to the above listed error. You can fix the error and try it again.\n");
      }
    } else {
      printError("The account was not removed from oidc-agent due to the above listed error. You can fix the error and try it again.\n");
    }
    clearFreeString(pairs[1].value); clearFreeString(pairs[0].value);
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief creates account from config file.
 * The config file is provided by the user. It might be a clientconfig file
 * created and encrypted by oidc-gen or an unencrypted file.
 * @param filename the absolute path of the account config file 
 * @return a pointer to the result oidc_account struct. Has to be freed after
 * usage using \f freeAccount
 */
struct oidc_account* accountFromFile(const char* filename) {
  char* inputconfig = readFile(filename);
  if(!inputconfig) {
    printError("Could not read config file: %s\n", oidc_serror());
    exit(EXIT_FAILURE);
  }
  syslog(LOG_AUTHPRIV|LOG_DEBUG, "Read config from user provided file: %s", inputconfig);
  struct oidc_account* account = getAccountFromJSON(inputconfig);
  if(!account) {
    char* encryptionPassword = NULL;
    int i;
    for(i=0; i<MAX_PASS_TRIES && account==NULL; i++) {
      syslog(LOG_AUTHPRIV|LOG_DEBUG, "Read config from user provided file: %s", inputconfig);
      encryptionPassword = promptPassword("Enter decryption Password for client config file: ");
      account = decryptAccountText(inputconfig, encryptionPassword);
      clearFreeString(encryptionPassword);
    }
    if(account!=NULL) {
      account_setRefreshToken(account, NULL); // currently not read correctly, there won't be any valid one in it
    }
  }
  clearFreeString(inputconfig);
  return account;
}

/**
 * @brief updates the issuer.config file.
 * If the issuer url is not already in the issuer.config file, it will be added.
 * @param issuer_url the issuer url to be added
 */
void updateIssuerConfig(const char* issuer_url) {
  char* issuers = readOidcFile(ISSUER_CONFIG_FILENAME);
  if(strcasestr(issuers, issuer_url)!=NULL) {
    clearFreeString(issuers);
    return;
  }
  char* new_issuers = oidc_sprintf("%s%s", issuers, issuer_url);
  clearFreeString(issuers);
  if(new_issuers == NULL) {
    syslog(LOG_AUTHPRIV|LOG_ERR, "%s", oidc_serror());
  } else {
    writeOidcFile(ISSUER_CONFIG_FILENAME, new_issuers);
    clearFreeString(new_issuers);
  }
}

/**
 * @brief encrypts and writes an account configuration.
 * @param text the json encoded account configuration text
 * @param suggestedPassword the suggestedPassword for encryption, won't be
 * displayed; can be NULL.
 * @param filepath an absolute path to the output file. Either filepath or
 * filename has to be given. The other one shall be NULL.
 * @param filename the filename of the output file. The output file will be
 * placed in the oidc dir. Either filepath or filename has to be given. The
 * other one shall be NULL.
 * @return an oidc_error code. oidc_errno is set properly.
 */
oidc_error_t encryptAndWriteConfig(const char* text, const char* hint, const char* suggestedPassword, const char* filepath, const char* oidc_filename) {
  initCrypt();
  char* encryptionPassword = getEncryptionPassword(hint, suggestedPassword, UINT_MAX); 
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

/**
 * @brief prompts the user and sets the account field using the provided
 * function.
 * @param account the account struct that will be updated
 * @param prompt_str the string used for prompting
 * @param set_callback the callback function for setting the account field
 * @param get_callback the callback function for getting the account field
 * @param passPrompt indicates if if prompting for a password. If 0 non password
 * prompt is used.
 * @param optional indicating if the field is optional or not.
 */
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

void promptAndSetIssuer(struct oidc_account* account) {
  if(!oidcFileDoesExist(ISSUER_CONFIG_FILENAME)) {
    promptAndSet(account, "Issuer%s%s%s: ", account_setIssuerUrl, account_getIssuerUrl, 0, 0);
  } else {
    useSuggestedIssuer(account); 
  }
  stringifyIssuerUrl(account);
}

void promptAndSetClientId(struct oidc_account* account) {
  promptAndSet(account, "Client_id%s%s%s: ", account_setClientId, account_getClientId, 0, 0);
}

void promptAndSetClientSecret(struct oidc_account* account) {
  promptAndSet(account, "Client_secret%s%s%s: ", account_setClientSecret, account_getClientSecret, 0, 0);
}

void promptAndSetScope(struct oidc_account* account) {
  if(!isValid(account_getScope(*account))) {
    char* defaultScope = oidc_sprintf("%s", DEFAULT_SCOPE);
    account_setScope(account, defaultScope);
  }
  promptAndSet(account, "Space delimited list of scopes%s%s%s: ", account_setScope, account_getScope, 0, 0);
}

void promptAndSetRefreshToken(struct oidc_account* account) {
  promptAndSet(account, "Refresh token%s%s%s: ", account_setRefreshToken, account_getRefreshToken, 0, 1);
}

void promptAndSetUsername(struct oidc_account* account) {
  promptAndSet(account, "Username%s%s%s: ", account_setUsername, account_getUsername, 0, 1);
}

void promptAndSetRedirectUris(struct oidc_account* account) {
  char* input = NULL;
  char* arr_str = arrToListString(account_getRedirectUris(*account), account_getRedirectUrisCount(*account), ' ', 1);
  do {
    input = prompt("Space separated redirect_uris%s: ", isValid(arr_str) ? arr_str : "");
    if(isValid(input)) {
      size_t size = listStringToArray(input, ' ', NULL);
      char** redirect_uris = calloc(sizeof(char*), size);
      listStringToArray(input, ' ', redirect_uris);
      account_setRedirectUris(account, redirect_uris, size);
    }
    clearFreeString(input);
    if(isValid(account_getRefreshToken(*account)) || (isValid(account_getUsername(*account)) && isValid(account_getPassword(*account)))) {
      break; //redirect_uris only required if no refresh token and no user credentials provided
    }
    clearFreeString(arr_str);
    arr_str = arrToListString(account_getRedirectUris(*account), account_getRedirectUrisCount(*account), ' ', 1);
  } while(!isValid(arr_str));
  clearFreeString(arr_str);
}

void promptAndSetPassword(struct oidc_account* account) {
  promptAndSet(account, "Password%s: ", account_setPassword, account_getPassword, 1, 1);
}

void promptAndSetCertPath(struct oidc_account* account, struct optional_arg cert_path) {
  if(cert_path.useIt && cert_path.str) {
      account_setCertPath(account, oidc_strcopy(cert_path.str));
      return;
  }  
  if(!isValid(account_getCertPath(*account))) {
    unsigned int i;
    for(i=0; i<sizeof(possibleCertFiles)/sizeof(*possibleCertFiles); i++) {
      if(fileDoesExist(possibleCertFiles[i])) {
        account_setCertPath(account, oidc_strcopy(possibleCertFiles[i]));
        break;
      }
    }
  }
  if(cert_path.useIt) {
  promptAndSet(account, "Cert Path%s%s%s: ", account_setCertPath, account_getCertPath, 0, 0);
  }
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

void useSuggestedIssuer(struct oidc_account* account) {
  char* fileContent = readOidcFile(ISSUER_CONFIG_FILENAME);
  int size = strCountChar(fileContent, '\n')+1;
  char* accounts[size];
  accounts[0] = strtok(fileContent, "\n");
  int i;
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
  if(isValid(account_getIssuerUrl(*account))) {
    fav = account_getIssuerUrl(*account);
  }
  for(i=0; i<size; i++) {
    printf(C_PROMPT "[%d] %s\n" C_RESET, i+1, accounts[i]); // printed indices starts at 1 for non nerd
  }

  while((i = promptIssuer(account, fav)) >= size) {
    printf(C_ERROR "input out of bound\n" C_RESET);
  } 
  if(i>=0) {
    char* iss = calloc(sizeof(char), strlen(accounts[i])+1);
    strcpy(iss, accounts[i]);
    struct oidc_issuer* issuer = calloc(sizeof(struct oidc_issuer), 1);
    issuer_setIssuerUrl(issuer, iss);
    account_setIssuer(account, issuer);
  }
  clearFreeString(fileContent);
}

int promptIssuer(struct oidc_account* account, const char* fav) {
  char* input = prompt("Issuer [%s]: ", fav);
  if(!isValid(input)) {
    char* iss = calloc(sizeof(char), strlen(fav)+1);
    strcpy(iss, fav);
    clearFreeString(input);
    struct oidc_issuer* issuer = calloc(sizeof(struct oidc_issuer), 1);
    issuer_setIssuerUrl(issuer, iss);
    account_setIssuer(account, issuer);
    return -1;
  } else if (isdigit(*input)) {
    int i = atoi(input);
    clearFreeString(input);
    i--; // printed indices starts at 1 for non nerds
    return i;
  } else {
    struct oidc_issuer* issuer = calloc(sizeof(struct oidc_issuer), 1);
    issuer_setIssuerUrl(issuer, input);
    account_setIssuer(account, issuer);
    return -1;
  }
}

void stringifyIssuerUrl(struct oidc_account* account) {
  int issuer_len = strlen(account_getIssuerUrl(*account));
  if(account_getIssuerUrl(*account)[issuer_len-1]!='/') {
    char* tmp = realloc(account_getIssuerUrl(*account), issuer_len+1+1);   
    if(NULL==tmp) {
      printError("realloc failed\n");
      exit(EXIT_FAILURE);
    }
    account->issuer->issuer_url = tmp; // don't use issuer_setsIssuerUrl here, because the free in it would double free because of realloc
    account_getIssuerUrl(*account)[issuer_len] = '/';
    issuer_len = strlen(account_getIssuerUrl(*account));
    account_getIssuerUrl(*account)[issuer_len] = '\0';
  }
}

char* encryptAccount(const char* json, const char* password) {
  char salt_hex[2*SALT_LEN+1] = {0};
  char nonce_hex[2*NONCE_LEN+1] = {0};
  unsigned long cipher_len = strlen(json) + MAC_LEN;

  char* cipher_hex = crypt_encrypt((unsigned char*)json, password, nonce_hex, salt_hex);
  char* fmt = "%lu:%s:%s:%s";
  char* write_it = oidc_sprintf(fmt, cipher_len, salt_hex, nonce_hex, cipher_hex);
  clearFreeString(cipher_hex);
  return write_it;
}

char* getEncryptionPassword(const char* forWhat, const char* suggestedPassword, unsigned int max_pass_tries) {
  char* encryptionPassword = NULL;
  unsigned int i;
  unsigned int max_tries = max_pass_tries==0 ? MAX_PASS_TRIES : max_pass_tries;
  for(i=0; i<max_tries; i++) {
    char* input = promptPassword("Enter encryption password for %s%s: ", forWhat, isValid(suggestedPassword) ? " [***]" : "");
    if(suggestedPassword && !isValid(input)) { // use same encryption password
      clearFreeString(input);
      encryptionPassword = calloc(sizeof(char), strlen(suggestedPassword)+1);
      strcpy(encryptionPassword, suggestedPassword);
      return encryptionPassword;
    } else {
      encryptionPassword = input;
      char* confirm = promptPassword("Confirm encryption Password: ");
      if(strcmp(encryptionPassword, confirm)!=0) {
        printf(C_ERROR "Encryption passwords did not match.\n" C_RESET);
        clearFreeString(confirm);
        clearFreeString(encryptionPassword);
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
  char* path = oidc_sprintf(path_fmt, iss, today, client_id);
  clearFreeString(today);
  clearFreeString(iss);


  if(oidcFileDoesExist(path)) {
    syslog(LOG_AUTHPRIV|LOG_DEBUG, "The clientconfig file already exists. Changing path.");
    int i = 0;
    char* newName = NULL;
    do {
      clearFreeString(newName);
      newName = oidc_sprintf("%s%d", path, i);
      i++;
    } while(oidcFileDoesExist(newName));
    clearFreeString(path);
    path = newName;
  }
  return path;
}

void gen_handleList() {
  list_t* list = getClientConfigFileList();
  char* str = listToDelimitedString(list, '\n');
  list_destroy(list);
  printf("The following client configuration files are usable:\n%s\n", str); 
  clearFreeString(str);
}

void gen_handlePrint(const char* file) {
  if(file==NULL || strlen(file) < 1) {
    printError("FILE not specified\n");
  }
  char* fileContent = NULL;
  if(file[0]=='/' || file[0]=='~') { //absolut path
    fileContent = readFile(file);
  } else { //file placed in oidc-dir
    fileContent = readOidcFile(file); 
  }
  char* password = NULL;
  unsigned char* decrypted = NULL;
  int i;
  for(i=0; i<MAX_PASS_TRIES && decrypted==NULL; i++) {
    password = promptPassword("Enter decryption Password for the passed file: ");
    decrypted = decryptFileContent(fileContent, password);
    clearFreeString(password);
  }
  clearFreeString(fileContent);
  if(decrypted==NULL) {
    exit(EXIT_FAILURE);
  }
  printf("%s\n", decrypted);
  clearFreeString((char*) decrypted);
}
