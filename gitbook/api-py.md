## liboidcagent-py
A `python` library for `oidc-agent` is available at
https://github.com/indigo-dc/liboidc-agent-py.

To use it in your `python` application install is with
```
pip install liboidcagent
```
and import it with:
``````python``
import liboidcagent as agent
```
### Requesting an Access Token For an Account Configuration
The following functions can be used to obtain an access token for a specific
account configuration from ```oidc-agent```. If you / your application does not
know which account configuration should be used, but you know for which provider
you need an access token you can also [request an access token for a
provider](#requesting-an-access-token-for-a-provider).

#### get_access_token
```python
def get_access_token(account_name, min_valid_period=0,
                     application_hint=None, scope=None)
``` 
This function requests an access token from oidc-agent for the ```account_name```
account configuration. The access token should have ```scope``` scopes and be
valid for at least ```minValidPeriod``` seconds. 

##### Parameters
- ```account_name``` is the shortname of the account configuration that should be
  used.
- If ```min_valid_period``` is
```0``` (default) no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. Can be omitted.
- ```application_hint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes. Can be omitted.
- If ```scope``` is None, the
default scopes for that account are used. So usually it is enough to use `None`
or to omitt this parameter. 

##### Return Value
The function returns only the access token. To additionally obtain other
information use [```get_token_response```](#get-token-response).
On failure an error object is returned.

##### Example
A complete example can look the following:
```python
token = agent.get_access_token(accountname, 60, "example-app")
print("Access token is: {}".format(token))
```
TODO TODO TODO
#### GetTokenResponse
```go
func GetTokenResponse(
      accountname string, minValidPeriod uint64, scope, applicationHint string) (
      resp TokenResponse, err error)
```
This function requests an access token from oidc-agent for the ```accountname```
account configuration. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```accountname``` is the shortname of the account configuration that should be
  used.
- If ```minValidPeriod``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is an empty string, the
default scopes for that account are used. So usually it is enough to use ```""```. 
- ```applicationHint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function returns an ```TokenResponse struct``` that contains the requested
access token, the url of the issuer that issued the token and the time when the
token expires.

The values can be accesed the following way:
```go
response, err := liboidcagent.GetTokenResponse(...)
response.Token      // access token
response.Issuer     // issuer url
response.ExpiresAt  // expiration time
```

##### Example
A complete example can look the following:
```go
resp, err := liboidcagent.GetTokenResponse(accountname, 60, "", "example-app")
if err != nil {
  fmt.Printf("%s\n", err)
  // Additional error handling
} else {
  fmt.Printf("Access token is: %s\n", resp.Token)
  fmt.Printf("Issuer url is: %s\n", resp.Issuer)
  fmt.Printf("The token expires at: %s\n", resp.ExpiresAt)
}
```

### Requesting an Access Token For a Provider
The following functions
can be used to obtain an access token for a specific OpenID Provider (issuer).
This might be useful for applications that only work with a specific provider
and therefore know the issuer for which they need an access token, but do not
require the user to provide an account configuration shortname.

#### GetAccessTokenByIssuerURL
```go
func GetAccessTokenByIssuerURL(
      issuerURL string, minValidPeriod uint64, scope, applicationHint string) (
      token string, err error)
```
This function requests an access token from `oidc-agent` for the provider with
```issuer_url```. The access token should have ```scope``` scopes and be valid for at least ```minValidPeriod``` seconds. 

##### Parameters
- ```issuerURL``` is the issuer url of the provider for which an access token
  should be obtained.
- If ```minValidPeriod``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is an empty string, the default scopes for that account are used. So usually it is enough to use ```""```. 
- ```applicationHint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function returns only the access token as a ```string``` and an error object. To additionally obtain other
information use
[```GetTokenResponseByIssuerURL```](#gettokenresponsebyissuerurl).
On failure an error object is returned.

##### Example
A complete example can look the following:
```go
token, err := liboidcagent.GetAccessTokenByIssuerURL("https://example.com/", 60, "", "example-app")
if err != nil {
  fmt.Printf("%s\n", err)
  // Additional error handling
} else {
  fmt.Printf("Access token is: %s\n", token)
}
```

#### GetTokenResponseByIssuerURL
```go
func GetTokenResponseByIssuerURL(
      issuer string, minValidPeriod uint64, scope, applicationHint string) (
      tokenResponse TokenResponse, err error)
```
This function requests an access token from oidc-agent for the provider with
```issuer_url```. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```issuer``` is the issuer url of the provider for which an access token
  should be obtained.
- If ```minValidPeriod``` is
```0``` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used. 
- If ```scope``` is an empty string, the
default scopes for that account are used. So usually it is enough to use ```""```. 
- ```applicationHint``` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.

##### Return Value
The function returns an ```TokenResponse struct``` that contains the requested
access token, the url of the issuer that issued the token and the time when the
token expires.

The values can be accesed the following way:
```go
response, err := liboidcagent.GetTokenResponse(...)
response.Token      // access token
response.Issuer     // issuer url
response.ExpiresAt  // expiration time
```

#### Example
A complete example can look the following:
```go
resp, err := liboidcagent.GetTokenResponseByIssuerURL("https://example.com", 60, "", "example-app")
	if err != nil {
		fmt.Printf("%s\n", err)
	} else {
		fmt.Printf("Access token is: %s\n", resp.Token)
		fmt.Printf("Issuer url is: %s\n", resp.Issuer)
		fmt.Printf("Expires at is: %s\n", resp.ExpiresAt)
	}
```
