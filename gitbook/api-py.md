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

### Error Handling
The library will raise an exception of type `OidcAgentError` if something goes
wrong.

Error Handling can be done the following way:
```python
try:
    print(agent.get_access_token(account_name))
except agent.OidcAgentError as e:
    print("ERROR oidc-agent: {}".format(e))
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

##### Example
A complete example can look the following:
```python
token = agent.get_access_token(account_name, 60, "example-app")
print("Access token is: {}".format(token))
```
#### get_token_response
```python
def get_token_response(account_name, min_valid_period=0,
                       application_hint=None, scope=None)
```
This function requests an access token from oidc-agent for the ```account_name```
account configuration. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

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
The function returns three values: the requested
access token, the url of the issuer that issued the token and the time when the
token expires.

The values can be accesed the following way:
```python
token, iss, exp = agent.get_token_response(account_name, 60, "example-app")
token      // access token
iss        // issuer url
exp        // expiration time

token_response = agent.get_token_response(account_name, 60, "example-app")
token_response[0]      // access token
token_response[1]      // issuer url
token_response[2]      // expiration time
```

##### Example
A complete example can look the following:
```python
token, iss, exp = agent.get_token_response(account_name, 60, "example-app")
print("Access token is: {}".format(token))
print("Access token issued by: {}".format(iss))
print("Access token expires at: {}".format(exp))
```

### Requesting an Access Token For a Provider
The following functions
can be used to obtain an access token for a specific OpenID Provider (issuer).
This might be useful for applications that only work with a specific provider
and therefore know the issuer for which they need an access token, but do not
require the user to provide an account configuration shortname.

#### get_access_token_by_issuer_url
```python
def get_access_token_by_issuer_url(issuer_url, min_valid_period=0,
                                   application_hint=None, scope=None)
```
This function requests an access token from `oidc-agent` for the provider with
```issuer_url```. The access token should have ```scope``` scopes and be valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```issuer_url``` is the issuer url of the provider for which an access token
  should be obtained.
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
information use
[```get_token_response_by_issuer_url```](#get-token-response-by-issuer-url).

##### Example
A complete example can look the following:
```python
token = agent.get_access_token_by_issuer_url("https://example.com", 60, "example-app")
print("Access token is: {}".format(token))
```

#### get_token_response_by_issuer_url
```python
def get_token_response_by_issuer_url(issuer_url, min_valid_period=0,
                                     application_hint=None, scope=None)
```
This function requests an access token from oidc-agent for the provider with
```issuer_url```. The access token should have ```scope``` scopes and be
valid for at least ```min_valid_period``` seconds. 

##### Parameters
- ```issuer_url``` is the issuer url of the provider for which an access token
  should be obtained.
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
The function returns three values: the requested
access token, the url of the provider that issued the token and the time when the
token expires.

The values can be accesed the following way:
```python
token, iss, exp = agent.get_token_response_by_issuer_url(issuer_url, 60, "example-app")
token      // access token
iss        // issuer url
exp        // expiration time

token_response = agent.get_token_response_by_issuer_url(issuer_url, 60, "example-app")
token_response[0]      // access token
token_response[1]      // issuer url
token_response[2]      // expiration time
```

##### Example
A complete example can look the following:
```python
token, iss, exp = agent.get_token_response_by_issuer_url("https://example.com", 60, "example-app")
print("Access token is: {}".format(token))
print("Access token issued by: {}".format(iss))
print("Access token expires at: {}".format(exp))
```

`
