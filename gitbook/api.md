# API
## liboidc-agent2
The C-API provides functions for getting an access token for a specific configuration as well as the
associated issuer. These functions are designed for easy usage. The C-API is available as a shared library through the ```liboidc-agent2``` package. The developement files (i.e. header-files) and the static library are included in the ```liboidc-agent-dev``` package.

### Requesting an Access Token
There are two functions for obtaining an access token from oidc-agent. However,
[```getAccessToken```](#getaccesstoken) is deprecated and [```getTokenResponse```](#gettokenresponse) should be used
instead.

#### getAccessToken
This function is deprecated and should not be used in new applications. Instead
use [```getTokenResponse```](#gettokenresponse).

#### getTokenResponse
```c
struct token_response getTokenResponse(const char*   accountname,
                                       unsigned long min_valid_period,
                                       const char*   scope,
                                       const char*   application_hint);
```
This function requests an access token from oidc-agent for the ```accountname```
account configuration. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

##### Parameters
- If ```min_valid_period``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is ```NULL```, the
default scopes for that account are used. So normally it is enough to use ```NULL```. 
- ```application_hint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function return an ```token_response struct``` that contains the requested
access token, the url of the issuer that issued the token and the time when the
token expires (in seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC)).

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

An complete example can look the following:
```c
struct token_response response = getTokenResponse(accountname, 60, NULL,
"example-app");
if(response.token == NULL) {
  oidcagent_perror();
  // Additional error handling
}
printf("Access token is: %s\n", response.token);
printf("Issuer url is: %s\n", response.issuer);
printf("Token expires at: %lu\n", response.expires_at);
secFreeTokenResponse(response);
```

### Error Handling
If an error occurs in any API function, ```oidc_errno``` is set to an error
code. An application might want to check this variable and perform specific
actions on some of the errors. A list of important error codes can be found at
[Error Codes](#error-codes); for all error codes refer to the ```oidc_error.h```
header file.

In most cases it is enough to print an error message to the user. For that usage
```liboidc-agent2``` provides some helperfunctions:
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

## IPC-API
Alternatively an application can directly communicate with the oidc-agent through UNIX domain sockets. The socket address can be obtained from the environment variable which is set by the agent (```OIDC_SOCK```). The request has to be sent json encoded. We use a UNIX domain socket of type ```SOCK_SEQPACKET```.

All Clients should ignore additional fields returned in a response from
oidc-agent, if the client does not understand these fields. Vice versa
also oidc-agent ignores fields that it does not understand.

The following fields and values have to be present for the different calls:

### Access Token:
#### Request
| field            | value                              | Requirement Level |
|------------------|------------------------------------|-------------------|
| request          | access_token                       | REQUIRED          |
| account          | &lt;account_shortname&gt;              | REQUIRED          |
| min_valid_period | &lt;min_valid_period&gt; [s]           | RECOMMENDED       |
| application_hint | &lt;application_name&gt;            | RECOMMENDED       |
| scope            | &lt;space delimited list of scopes&gt; | OPTIONAL          |

example:
```
{"request":"access_token", "account":"iam", "min_valid_period":60,
"application_hint":"example_application", "scope":"openid profile phone"}
```

#### Response
| field        | value          |
|--------------|----------------|
| status       | success        |
| access_token | &lt;access_token&gt; |
| issuer       | &lt;issuer_url&gt; |
| expires_at       | &lt;expiration time&gt; |

example:
```
{"status":"success", "access_token":"token1234", "issuer":"https:example.com/",
"expires_at":1541517118}
```

#### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | &lt;error_description&gt; |

example:
```
{"status":"failure", "error":"Account not loaded"}
```

### List of Accounts:
The ability to retrieve the list of currently loaded accounts was removed with version 2.0.0.


