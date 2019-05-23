## liboidc-agent3
The C-API provides functions for getting an access token for a specific configuration as well as the
associated issuer. These functions are designed for easy usage. The C-API is available as a shared library through the ```liboidc-agent3``` package. The developement files (i.e. header-files) and the static library are included in the ```liboidc-agent-dev``` package.

### Requesting an Access Token For an Account Configuration
The following functions can be used to obtain an access token for a specific
account configuration from ```oidc-agent```. If you / your application does not
know which account configuration should be used, but you know for which provider
you need an access token you can also [request an access token for a
provider](#requesting-an-access-token-for-a-provider).

#### getAccessToken
This function is deprecated and should not be used in new applications. Instead
use [```getAccessToken2```](#getaccesstoken2) or [```getTokenResponse```](#gettokenresponse).

#### getAccessToken2
```c
char* getAccessToken2(const char* accountname, time_t min_valid_period,
                      const char* scope, const char* application_hint)
```
This function requests an access token from oidc-agent for the ```accountname```
account configuration. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```accountname``` is the shortname of the account configuration that should be
  used.
- If ```min_valid_period``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is ```NULL```, the
default scopes for that account are used. So usually it is enough to use ```NULL```. 
- ```application_hint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function returns only the access token as a ```char*```. To additionally obtain other
information use [```getTokenResponse```](#gettokenresponse).
After usage the return value MUST be freed using ```secFree```.

On failure ```NULL``` is returned and ```oidc_errno``` is set
(see [Error Handling](#error-handling)). 

##### Example
A complete example can look the following:
```c
char* token = getAccessToken2(accountname, 60, NULL,
"example-app");
if(token == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("Access token is: %s\n", token);
  secFree(token);
}
```
#### getTokenResponse
```c
struct token_response getTokenResponse(const char*   accountname,
                                       time_t        min_valid_period,
                                       const char*   scope,
                                       const char*   application_hint)
```
This function requests an access token from oidc-agent for the ```accountname```
account configuration. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```accountname``` is the shortname of the account configuration that should be
  used.
- If ```min_valid_period``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is ```NULL```, the
default scopes for that account are used. So usually it is enough to use ```NULL```. 
- ```application_hint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function returns an ```token_response struct``` that contains the requested
access token, the url of the issuer that issued the token and the time when the
token expires (in seconds since the Epoch, ```1970-01-01 00:00:00 +0000 (UTC)```).

The values can be accesed the following way:
```c
struct token_response response = getTokenResponse(...);
response.token      // access token
response.issuer     // issuer url
response.expires_at // expiration time
```

After usage the return value MUST be freed using ```secFreeTokenResponse```.

On failure ```response.token``` will be ```NULL``` and ```oidc_errno``` is set
(see [Error Handling](#error-handling)). So applications should check
```response.token``` before accessing any of the token response values.

##### Example
A complete example can look the following:
```c
struct token_response response = getTokenResponse(accountname, 60, NULL,
"example-app");
if(response.token == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("Access token is: %s\n", response.token);
  printf("Issuer url is: %s\n", response.issuer);
  printf("Token expires at: %lu\n", response.expires_at);
  secFreeTokenResponse(response);
}
```

### Requesting an Access Token For a Provider
The ```getAccessTokenForIssuer``` and ```getTokenResponseForIssuer``` methods
can be used to obtain an access token for a specific OpenID Provider (issuer).
This might be useful for applications that only work with a specific provider
and therefore know the issuer for which they need an access token, but do not
require the user to provide an account configuration shortname.

#### getAccessTokenForIssuer
```c
char* getAccessTokenForIssuer(const char* issuer_url, time_t min_valid_period,
                              const char* scope, const char* application_hint)
```
This function requests an access token from oidc-agent for the provider with
```issuer_url```. The access token should have ```scope``` scopes and be valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```issuer_url``` is the issuer url of the provider for which an access token
  should be obtained.
- If ```min_valid_period``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is ```NULL```, the default scopes for that account are used. So usually it is enough to use ```NULL```. 
- ```application_hint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function returns only the access token as a ```char*```. To additionally obtain other
information use [```getTokenResponseForIssuer```](#gettokenresponseforissuer).
After usage the return value MUST be freed using ```secFree```.

On failure ```NULL``` is returned and ```oidc_errno``` is set
(see [Error Handling](#error-handling)). 

##### Example
A complete example can look the following:
```c
char* token = getAccessTokenForIssuer("https://example.com/", 60, NULL,
"example-app");
if(token == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("Access token is: %s\n", token);
  secFree(token);
}
```

#### getTokenResponseForIssuer
```c
struct token_response getTokenResponseForIssuer(const char* issuer_url,
                                                time_t      min_valid_period,
                                                const char* scope,
                                                const char* application_hint)
```
This function requests an access token from oidc-agent for the the provider with
```issuer_url```. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```issuer_url``` is the issuer url of the provider for which an access token
  should be obtained.
- If ```min_valid_period``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is ```NULL```, the
default scopes for that account are used. So usually it is enough to use ```NULL```. 
- ```application_hint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function returns an ```token_response struct``` that contains the requested
access token, the url of the issuer that issued the token and the time when the
token expires (in seconds since the Epoch, ```1970-01-01 00:00:00 +0000 (UTC)```).

The values can be accesed the following way:
```c
struct token_response response = getTokenResponse(...);
response.token      // access token
response.issuer     // issuer url
response.expires_at // expiration time
```

After usage the return value MUST be freed using ```secFreeTokenResponse```.

On failure ```response.token``` will be ```NULL``` and ```oidc_errno``` is set
(see [Error Handling](#error-handling)). So applications should check
```response.token``` before accessing any of the token response values.

##### Example
A complete example can look the following:
```c
struct token_response response = getTokenResponseForIssuer(
  "https://example.com/", 60, NULL, "example-app");
if(response.token == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("Access token is: %s\n", response.token);
  printf("Issuer url is: %s\n", response.issuer);
  printf("Token expires at: %lu\n", response.expires_at);
  secFreeTokenResponse(response);
}
```

### Error Handling
If an error occurs in any API function, ```oidc_errno``` is set to an error
code. An application might want to check this variable and perform specific
actions on some of the errors. A list of important error codes can be found at
[Error Codes](#error-codes); for all error codes refer to the ```oidc_error.h```
header file.

In most cases it is enough to print an error message to the user. For that usage
```liboidc-agent3``` provides some helperfunctions:
```c
void oidcagent_perror();
char* oidcagent_serror();
```

```oidcagent_perror()``` can be used similar to ```perror()``` and prints the
an error message describing the last occured error to ```stderr```.
```oidcagent_serror()``` returns the string that describes the error without
printing it. The return string MUST NOT be freed. This function behaves similar
to ```strerror(errno)```.

#### Error Codes
| error code | explanation |
|------------|-------------|
|OIDC_SUCCESS | success - no error | 
| OIDC_EERROR | general error - check the error string|
| OIDC_ENOACCOUNT | the account is not loaded|
|OIDC_EOIDC | an error related to OpenID Connect happened - check the error string|
| OIDC_EENVVAR | the environment variable used to locate the agent is not set|
| OIDC_ECONSOCK | could not connect to the oidc-agent socket - most likely the agent is not running|
| OIDC_ELOCKED| the agent is locked and first has to be unlocked by the user|
| OIDC_EFORBIDDEN|the user forbid this action|
| OIDC_EPASS | wrong password - might occur if the account was not loaded and the user entered a wrong password in the autoload prompt|
