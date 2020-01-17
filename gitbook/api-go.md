## liboidcagent-go
A `go` library for `oidc-agent` is available at
https://github.com/indigo-dc/liboidcagent-go.

To use it in your `go` application include:
```go
import "github.com/indigo-dc/liboidcagent-go/liboidcagent"
```
### Requesting an Access Token For an Account Configuration
The following functions can be used to obtain an access token for a specific
account configuration from `oidc-agent`. If you / your application does not
know which account configuration should be used, but you know for which provider
you need an access token you can also [request an access token for a
provider](#requesting-an-access-token-for-a-provider).

#### GetAccessToken
This function is deprecated and should not be used in new applications. Use
[`GetAccessToken2`](#getaccesstoken2) or [`GetTokenResponse2`](#gettokenresponse2) instead.

#### GetAccessToken2
```go
func GetAccessToken2(
      accountname string, minValidPeriod uint64, scope, applicationHint, audience string) (
      token string, err error)
```
This function requests an access token from oidc-agent for the `accountname`
account configuration. The access token should have `scope` scopes, be
valid for at least `minValidPeriod` seconds, and have the `audience` audience.

##### Parameters
- `accountname` is the shortname of the account configuration that should be
  used.
- If `minValidPeriod` is
`0` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used.
- If `scope` is an empty string, the
default scopes for that account are used. So usually it is enough to use `""`.
- `applicationHint` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.
- If `audience` is an empty string, no special audience is requested for this access
    token. This parameter is used to request an access token with a specific audience.

##### Return Value
The function returns only the access token as a `string` and an error object. To additionally obtain other
information use [`GetTokenResponse2`](#gettokenresponse2).
On failure an error object is returned.

##### Example
A complete example can look the following:
```go
token, err := liboidcagent.GetAccessToken2(accountname, 60, "", "example-app", "")
if err != nil {
  fmt.Printf("%s\n", err)
  // Additional error handling
} else {
  fmt.Printf("Access token is: %s\n", token)
}
```

#### GetTokenResponse
This function is deprecated and should not be used in new applications. Use
[`GetTokenResponse2`](#gettokenresponse2) instead.

#### GetTokenResponse2
```go
func GetTokenResponse2(
      accountname string, minValidPeriod uint64, scope, applicationHint, audience string) (
      resp TokenResponse, err error)
```
This function requests an access token from oidc-agent for the `accountname`
account configuration. The access token should have `scope` scopes, be
valid for at least `min_valid_period` seconds, and have the `audience` audience.

##### Parameters
- `accountname` is the shortname of the account configuration that should be
  used.
- If `minValidPeriod` is
`0` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used.
- If `scope` is an empty string, the
default scopes for that account are used. So usually it is enough to use `""`.
- `applicationHint` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.
- If `audience` is an empty string, no special audience is requested for this access
    token. This parameter is used to request an access token with a specific audience.

##### Return Value
The function returns an `TokenResponse struct` that contains the requested
access token, the url of the issuer that issued the token and the time when the
token expires.

The values can be accessed the following way:
```go
response, err := liboidcagent.GetTokenResponse2(...)
response.Token      // access token
response.Issuer     // issuer url
response.ExpiresAt  // expiration time
```

##### Example
A complete example can look the following:
```go
resp, err := liboidcagent.GetTokenResponse2(accountname, 60, "", "example-app", "")
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
This function is deprecated and should not be used in new applications. Use
[`GetAccessTokenByIssuerURL2`](#getaccesstokenbyissuerurl2) or [`GetTokenResponseByIssuerURL2`](#gettokenresponsebyissuerurl2) instead.

#### GetAccessTokenByIssuerURL2
```go
func GetAccessTokenByIssuerURL2(
      issuerURL string, minValidPeriod uint64, scope, applicationHint, audience string) (
      token string, err error)
```
This function requests an access token from `oidc-agent` for the provider with
`issuerURL`. The access token should have `scope` scopes, be valid for at least `minValidPeriod` seconds, and have the `audience` audience.

##### Parameters
- `issuerURL` is the issuer url of the provider for which an access token
  should be obtained.
- If `minValidPeriod` is
`0` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used.
- If `scope` is an empty string, the default scopes for that account are used. So usually it is enough to use `""`.
- `applicationHint` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.
- If `audience` is an empty string, no special audience is requested for this access
    token. This parameter is used to request an access token with a specific audience.

##### Return Value
The function returns only the access token as a `string` and an error object. To additionally obtain other
information use
[`GetTokenResponseByIssuerURL2`](#gettokenresponsebyissuerurl2).
On failure an error object is returned.

##### Example
A complete example can look the following:
```go
token, err := liboidcagent.GetAccessTokenByIssuerURL2("https://example.com/", 60, "", "example-app", "")
if err != nil {
  fmt.Printf("%s\n", err)
  // Additional error handling
} else {
  fmt.Printf("Access token is: %s\n", token)
}
```

#### GetTokenResponseByIssuerURL
This function is deprecated and should not be used in new applications. Use
[`GetTokenResponseByIssuerURL2`](#gettokenresponsebyissuerurl2) instead.

#### GetTokenResponseByIssuerURL2
```go
func GetTokenResponseByIssuerURL2(
      issuerURL string, minValidPeriod uint64, scope, applicationHint, audience string) (
      tokenResponse TokenResponse, err error)
```
This function requests an access token from oidc-agent for the provider with
`issuerURL`. The access token should have `scope` scopes, be
valid for at least `minValidPeriod` seconds, and have the `audience` audience.

##### Parameters
- `issuerURL` is the issuer url of the provider for which an access token
  should be obtained.
- If `minValidPeriod` is
`0` no guarantee about the validity of the token can be made; it is possible
that it expires before it can be used.
- If `scope` is an empty string, the
default scopes for that account are used. So usually it is enough to use `""`.
- `applicationHint` should be the name of the application that
requests an access token. This string might be displayed to the user for
authorization purposes.
- If `audience` is an empty string, no special audience is requested for this access
    token. This parameter is used to request an access token with a specific audience.

##### Return Value
The function returns an `TokenResponse struct` that contains the requested
access token, the url of the issuer that issued the token and the time when the
token expires.

The values can be accesed the following way:
```go
response, err := liboidcagent.GetTokenResponseByIssuerURL2(...)
response.Token      // access token
response.Issuer     // issuer url
response.ExpiresAt  // expiration time
```

#### Example
A complete example can look the following:
```go
resp, err := liboidcagent.GetTokenResponseByIssuerURL2("https://example.com", 60, "", "example-app", "")
if err != nil {
  fmt.Printf("%s\n", err)
} else {
  fmt.Printf("Access token is: %s\n", resp.Token)
  fmt.Printf("Issuer url is: %s\n", resp.Issuer)
  fmt.Printf("Expires at is: %s\n", resp.ExpiresAt)
}
```
