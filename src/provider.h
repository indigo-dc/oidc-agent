#ifndef PROVIDER_H
#define PROVIDER_H


struct token {
  char* access_token;
  unsigned long token_expires_at;
};

struct oidc_provider {
  char* issuer;
  char* name;                           // mandatory in config file
  char* client_id;                      // mandatory in config file
  char* client_secret;                  // mandatory in config file
  char* configuration_endpoint;         // mandatory in config file
  char* token_endpoint;                 // retrieved from configuration_endpoint
  char* username;                       // optional in config file
  char* password;
  char* refresh_token;                  // optional in config file
  struct token token;
};


#endif // PROVIDER_H

