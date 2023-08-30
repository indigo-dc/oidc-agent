## liboidc-agent5

The C-API provides functions for getting an access token for a specific configuration as well as the associated issuer.
These functions are designed for easy usage. The C-API is available as a shared library through the `liboidc-agent5`
package. The developement files (i.e. header-files) and the static library are included in the `liboidc-agent-dev`
package.

The library depends on `libsodium` therefore the `-lsodium` linker flag must be included when linking `liboidc-agent`.
If the library was build with `liblist`
then `-llist` must be included. If the library was build with `libcjson` then
`-lcjson` must be included. On modern distros this is usually the case.

### Requesting an Access Token For an Account Configuration

The following functions can be used to obtain an access token for a specific account configuration from `oidc-agent`. If
you / your application does not know which account configuration should be used, but you know for which provider you
need an access token you can also [request an access token for a provider](#requesting-an-access-token-for-a-provider).

#### getAccessToken

It is recommended to use [`getAgentTokenResponse`](#getagenttokenresponse) instead.

```c
char* getAccessToken(const char* accountname, time_t min_valid_period,
                     const char* scope, const char* application_hint,
                     const char* audience)
```

This function requests an access token from oidc-agent for the `accountname`
account configuration. The access token should have `scope` scopes, be valid for at least `min_valid_period` seconds,
and have the `audience` audience.

##### Parameters

- `accountname` is the shortname of the account configuration that should be used.
- If `min_valid_period` is
  `0` no guarantee about the validity of the token can be made; it is possible that it expires before it can be used.
- If `scope` is `NULL`, the default scopes for that account are used. So usually it is enough to use `NULL`.
- `application_hint` should be the name of the application that requests an access token. This string might be displayed
  to the user for authorization purposes.
- If `audience` is `NULL`, no special audience is requested for this access token. This parameter is used to request an
  access token with a specific audience.

##### Return Value

The function returns only the access token as a `char*`. To additionally obtain other information
use [`getAgentTokenResponse`](#getagenttokenresponse). After usage the return value MUST be freed using `secFree`.

On failure `NULL` is returned and `oidc_errno` is set
(see [Error Handling](#error-handling)).

##### Example

A complete example can look the following:

```c
char* token = getAccessToken(accountname, 60, NULL,
"example-app", NULL);
if(token == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("Access token is: %s\n", token);
  secFree(token);
}
```

#### getAgentTokenResponse

```c
struct agent_response getAgentTokenResponse(const char* accountname,
                                        time_t      min_valid_period,
                                        const char* scope,
                                        const char* application_hint,
                                        const char* audience)
```

This function requests an access token from oidc-agent for the `accountname`
account configuration. The access token should have `scope` scopes, be valid for at least `min_valid_period` seconds,
and have the `audience` audience.

##### Parameters

- `accountname` is the shortname of the account configuration that should be used.
- If `min_valid_period` is
  `0` no guarantee about the validity of the token can be made; it is possible that it expires before it can be used.
- If `scope` is `NULL`, the default scopes for that account are used. So usually it is enough to use `NULL`.
- `application_hint` should be the name of the application that requests an access token. This string might be displayed
  to the user for authorization purposes.
- If `audience` is `NULL`, no special audience is requested for this access token. This parameter is used to request an
  access token with a specific audience.

##### Return Value

The function returns an `agent_response struct`. The `type` element indicates which type is returned, i.e. if an error
occurred. On success the response has a `token_response struct` that contains the requested access token, the url of the
issuer that issued the token and the time when the token expires (in seconds since the
Epoch, `1970-01-01 00:00:00 +0000 (UTC)`).

The values can be accessed the following way:

```c
struct agent_response response = getAgentTokenResponse(...);
if (response.type == AGENT_RESPONSE_TYPE_TOKEN) { // assert that we actually have a token response
    struct token_response tok_res = response.token_response;
    tok_res.token      // access token
    tok_res.issuer     // issuer url
    tok_res.expires_at // expiration time
}
```

**After usage the return value MUST be freed using `secFreeAgentResponse`.**

On failure `response.type` will be `AGENT_RESPONSE_TYPE_ERROR` and `response.error_response` can be accessed
(see [Error Handling](#error-handling)). So applications should check
`response.type` before accessing any of the token response values.

##### Example

A complete example can look the following:

```c
struct agent_response response = getAgentTokenResponse(accountname, 60, NULL,
"example-app", NULL);
if(response.type == AGENT_RESPONSE_TYPE_ERROR) {
    oidcagent_printErrorResponse(response.error_response);
    // Additional error handling
} else {
    struct token_response tok_res = response.token_response;
    printf("Access token is: %s\n", tok_res.token);
    printf("Issuer url is: %s\n", tok_res.issuer);
    printf("Token expires at: %lu\n", tok_res.expires_at);
}
secFreeAgentResponse(response);
```

### Requesting an Access Token For a Provider

The `getAccessTokenForIssuer` and `getAgentTokenResponseForIssuer` methods can be used to obtain an access token for a
specific OpenID Provider (issuer). This is useful for applications that only work with a specific provider and therefore
know the issuer for which they need an access token, but do not require the user to provide an account configuration
shortname.

#### getAccessTokenForIssuer

```c
char* getAccessTokenForIssuer(const char* issuer_url, time_t min_valid_period,
                              const char* scope, const char* application_hint,
                              const char* audience)
```

This function requests an access token from oidc-agent for the provider with
`issuer_url`. The access token should have `scope` scopes, be valid for at least `min_valid_period` seconds, and have
the `audience` audience.

##### Parameters

- `issuer_url` is the issuer url of the provider for which an access token should be obtained.
- If `min_valid_period` is
  `0` no guarantee about the validity of the token can be made; it is possible that it expires before it can be used.
- If `scope` is `NULL`, the default scopes for that account are used. So usually it is enough to use `NULL`.
- `application_hint` should be the name of the application that requests an access token. This string might be displayed
  to the user for authorization purposes.
- If `audience` is `NULL`, no special audience is requested for this access token. This parameter is used to request an
  access token with a specific audience.

##### Return Value

The function returns only the access token as a `char*`. To additionally obtain other information
use [`getAgentTokenResponseForIssuer`](#getagenttokenresponseforissuer). After usage the return value MUST be freed
using `secFree`.

On failure `NULL` is returned and `oidc_errno` is set
(see [Error Handling](#error-handling)).

##### Example

A complete example can look the following:

```c
char* token = getAccessTokenForIssuer("https://example.com/", 60, NULL,
"example-app", NULL);
if(token == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("Access token is: %s\n", token);
  secFree(token);
}
```

#### getAgentTokenResponseForIssuer

```c
struct agent_response getAgentTokenResponseForIssuer(const char* issuer_url,
                                                 time_t      min_valid_period,
                                                 const char* scope,
                                                 const char* application_hint,
                                                 const char* audience)
```

This function requests an access token from oidc-agent for the the provider with
`issuer_url`. The access token should have `scope` scopes, be valid for at least `min_valid_period` seconds, and have
the `audience` audience.

##### Parameters

- `issuer_url` is the issuer url of the provider for which an access token should be obtained.
- If `min_valid_period` is
  `0` no guarantee about the validity of the token can be made; it is possible that it expires before it can be used.
- If `scope` is `NULL`, the default scopes for that account are used. So usually it is enough to use `NULL`.
- `application_hint` should be the name of the application that requests an access token. This string might be displayed
  to the user for authorization purposes.
- If `audience` is `NULL`, no special audience is requested for this access token. This parameter is used to request an
  access token with a specific audience.

##### Return Value

The function returns an `agent_response struct`. The `type` element indicates which type is returned, i.e. if an error
occurred. On success the response has a `token_response struct` that contains the requested access token, the url of the
issuer that issued the token and the time when the token expires (in seconds since the
Epoch, `1970-01-01 00:00:00 +0000 (UTC)`).

The values can be accessed the following way:

```c
struct agent_response response = getAgentTokenResponseForIssuer(...);
if (response.type == AGENT_RESPONSE_TYPE_TOKEN) { // assert that we actually have a token response
    struct token_response tok_res = response.token_response;
    tok_res.token      // access token
    tok_res.issuer     // issuer url
    tok_res.expires_at // expiration time
}
```

**After usage the return value MUST be freed using `secFreeAgentResponse`.**

On failure `response.type` will be `AGENT_RESPONSE_TYPE_ERROR` and `response.error_response` can be accessed
(see [Error Handling](#error-handling)). So applications should check
`response.type` before accessing any of the token response values.

##### Example

A complete example can look the following:

```c
struct agent_response response = getAgentTokenResponseForIssuer("https://oidc.example.com", 60, NULL,
"example-app", NULL);
if(response.type == AGENT_RESPONSE_TYPE_ERROR) {
    oidcagent_printErrorResponse(response.error_response);
    // Additional error handling
} else {
    struct token_response tok_res = response.token_response;
    printf("Access token is: %s\n", tok_res.token);
    printf("Issuer url is: %s\n", tok_res.issuer);
    printf("Token expires at: %lu\n", tok_res.expires_at);
}
secFreeAgentResponse(response);
```

### Requesting a Mytoken

#### getAgentMytokenResponse

```c
struct agent_response getAgentMytokenResponse(const char* accountname,
                                        const char* mytoken_profile,
                                        const char* application_hint)
```

This function requests mytoken from oidc-agent for the `accountname`
mytoken account configuration. The mytoken should have the properties defined by the passed `mytoken_profile`.

##### Parameters

- `accountname` is the shortname of the account configuration that should be used.
- `mytoken_profile` is a [mytoken profile](https://mytoken-docs.data.kit.edu/concepts/profiles) describing the
  properties of the requested mytoken.
- `application_hint` should be the name of the application that requests the mytoken. This string might be displayed to
  the user for authorization purposes.

##### Return Value

The function returns an `agent_response struct`. The `type` element indicates which type is returned, i.e. if an error
occurred. On success the response has a `mytoken_response struct` that contains the requested mytoken as well as several
additional information.

The values can be accessed the following way:

```c
struct agent_response response = getAgentMytokenResponse(...);
if (response.type == AGENT_RESPONSE_TYPE_MYTOKEN) { // assert that we actually have a mytoken response
    struct mytoken_response tok_res = response.mytoken_response;
    tok_res.token      // mytoken
    tok_res.token_type;  // Type of the mytoken as returned from the mytoken server
    tok_res.mytoken_issuer; // Issuer url of the mytoken server that issued the mytoken
    tok_res.oidc_issuer; // Issuer url of the OpenID Provider that issues the access tokens for this mytoken
    tok_res.restrictions;           // Mytoken restrictions as JSON
    tok_res.capabilities;           // The mytokens capabilities as JSON Array
    tok_res.rotation;               // The rotation policy for this mytoken as JSON
    tok_res.expires_at; // The time when this mytoken expires

}
```

**After usage the return value MUST be freed using `secFreeAgentResponse`.**

On failure `response.type` will be `AGENT_RESPONSE_TYPE_ERROR` and `response.error_response` can be accessed
(see [Error Handling](#error-handling)). So applications should check
`response.type` before accessing any of the token response values.

##### Example

A complete example can look the following:

```c
struct agent_response response = getAgentMytokenResponse(accountname, NULL, "example-app");
if(response.type == AGENT_RESPONSE_TYPE_ERROR) {
    oidcagent_printErrorResponse(response.error_response);
    // Additional error handling
} else {
    struct mytoken_response tok_res = response.mytoken_response;
    printf("Mytoken is: %s\n", tok_res.token);
    printf("Useable at: %s\n", tok_res.mytoken_issuer);
    printf("For provider: %lu\n", tok_res.oidc_issuer);
}
secFreeAgentResponse(response);
```

#### getMytoken

```c
char* getMytoken(const char* accountname, const char* mytoken_profile, const char* application_hint)
```

This function requests mytoken from oidc-agent for the `accountname`
mytoken account configuration. The mytoken should have the properties defined by the
passed [`mytoken_profile`](https://mytoken-docs.data.kit.edu/concepts/profiles/).

##### Parameters

- `accountname` is the shortname of the account configuration that should be used.
- `mytoken_profile` is a [mytoken profile](https://mytoken-docs.data.kit.edu/concepts/profiles) describing the
  properties of the requested mytoken.
- `application_hint` should be the name of the application that requests the mytoken. This string might be displayed to
  the user for authorization purposes.

##### Return Value

The function returns only the mytoken as a `char*`. To additionally obtain other information
use [`getAgentMytokenResponse`](#getagentmytokenresponse). After usage the return value MUST be freed using `secFree`.

On failure `NULL` is returned and `oidc_errno` is set
(see [Error Handling](#error-handling)).

##### Example

A complete example can look the following:

```c
char* token = getMytoken(accountname, NULL, "example-app");
if(token == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("Mytoken is: %s\n", token);
  secFree(token);
}
```

### Getting a List of Loaded Accounts

The `getLoadedAccountsList` and `getAgentLoadedAccountsListResponse` functions can be used to obtain a list of account
configurations that are currently loaded in the agent. This can be used to show users a list from which they can choose
an account config to use. It is not necessary to check if an account is loaded through this function before requesting
tokens for it.

LIB_PUBLIC char* getLoadedAccountsList(); LIB_PUBLIC struct agent_response getAgentLoadedAccountsListResponse();

#### getAgentLoadedAccountsListResponse

```c
struct agent_response getAgentLoadedAccountsListResponse()
```

This function requests the list of currently loaded accounts from oidc-agent.

##### Return Value

The function returns an `agent_response struct`. The `type` element indicates which type is returned, i.e. if an error
occurred. On success the response has a `loaded_accounts_response struct` that contains the loaded accounts.

The value can be accessed the following way:

```c
struct agent_response response = getAgentLoadedAccountsListResponse(...);
if (response.type == AGENT_RESPONSE_TYPE_ACCOUNTS) { // assert that we actually have a accounts response
    struct loaded_accounts_response acc_res = response.loaded_accounts_response;
    acc_res.accounts      // loaded accounts as a space delimited string
}
```

**After usage the return value MUST be freed using `secFreeAgentResponse`.**

On failure `response.type` will be `AGENT_RESPONSE_TYPE_ERROR` and `response.error_response` can be accessed
(see [Error Handling](#error-handling)). So applications should check
`response.type` before accessing any of the token response values.

##### Example

A complete example can look the following:

```c
struct agent_response response = getAgentLoadedAccountsListResponse();
if(response.type == AGENT_RESPONSE_TYPE_ERROR) {
    oidcagent_printErrorResponse(response.error_response);
    // Additional error handling
} else {
    struct token_response acc_res = response.loaded_accounts_response;
    printf("The following accounts are loaded: %s\n", acc_res.accounts);
}
secFreeAgentResponse(response);
```

#### getLoadedAccountsList

```c
char* getLoadedAccountsList()
```

This function requests the list of currently loaded accounts from oidc-agent.

##### Return Value

The function returns the loaded accounts as a space delimited string `char*`. To additionally obtain other information (
extended error information)
use [`getAgentLoadedAccountsListResponse`](#getagentloadedaccountslistresponse). After usage the return value MUST be
freed using `secFree`.

On failure `NULL` is returned and `oidc_errno` is set
(see [Error Handling](#error-handling)).

##### Example

A complete example can look the following:

```c
char* accounts = getLoadedAccountsList();
if(accounts == NULL) {
  oidcagent_perror();
  // Additional error handling
} else {
  printf("The following accounts are loaded: %s\n", accounts);
  secFree(accounts);
}
```

### Error Handling

Since version `4.2.0` it is recommended to use functions that return an `agent_response struct`. This approach is
described in [Using the Error Response Structure](#using-the-error-response-structure). For functions that do not return
an `agent_response struct` [`oidc_errno`](#using-oidc_errno) must be used. This approach can also be used in addition to
the `error_response struct`.

#### Using the Error Response Structure

Since version `4.2.0` it is recommended to use functions that return an `agent_response struct`. This struct can hold
either a `token_response` or an `agent_error_response` depending on the success of the call.
The `agent_error_response struct` holds an error message and MIGHT additionally hold a help message (however, the help
message might also be `NULL`). If the help message is available it SHOULD be displayed to the user, since it gives
useful information how the user can solve the problem.

Before accessing the `agent_error_response struct` in an `agent_response` one MUST ensure that the `agent_response.type`
is `AGENT_RESPONSE_TYPE_ERROR`. This is also how one checks for the presence of an error.

```c
struct agent_response response = getAgentTokenResponse(...);
if (response.type == AGENT_RESPONSE_TYPE_ERROR) {
    // error
    struct agent_error_response err_res = response.error_response;
    err_res.error // the error message
    err_res.help // the help message (before using it assert that != NULL
} else {
    // success
}
```

`liboidcagent4` also provides a helper function to easily print an `agent_error_response`:

```c
oidcagent_printErrorResponse(response.error_response);
```

#### Using `oidc_errno`

If an error occurs in any API function, `oidc_errno` is set to an error code. An application might want to check this
variable and perform specific actions on some of the errors. A list of important error codes can be found at
[Error Codes](#error-codes); for all error codes refer to the `oidc_error.h`
header file.

In most cases it is enough to print an error message to the user. For that usage
`liboidc-agent4` provides some helper functions:

```c
void oidcagent_perror();
char* oidcagent_serror();
```

`oidcagent_perror()` can be used similar to `perror()` and prints an error message describing the last occurred error
to `stderr`.
`oidcagent_serror()` returns the string that describes the error without printing it. The return string MUST NOT be
freed. This function behaves similar to `strerror(errno)`.

#### Error Codes

| error code      | explanation                                                                                                             |
|-----------------|-------------------------------------------------------------------------------------------------------------------------|
| OIDC_SUCCESS    | success - no error                                                                                                      | 
| OIDC_EERROR     | general error - check the error string                                                                                  |
| OIDC_ENOACCOUNT | the account is not loaded                                                                                               |
| OIDC_EOIDC      | an error related to OpenID Connect happened - check the error string                                                    |
| OIDC_EENVVAR    | the environment variable used to locate the agent is not set                                                            |
| OIDC_ECONSOCK   | could not connect to the oidc-agent socket - most likely the agent is not running                                       |
| OIDC_ELOCKED    | the agent is locked and first has to be unlocked by the user                                                            |
| OIDC_EFORBIDDEN | the user forbid this action                                                                                             |
| OIDC_EPASS      | wrong password - might occur if the account was not loaded and the user entered a wrong password in the autoload prompt |
