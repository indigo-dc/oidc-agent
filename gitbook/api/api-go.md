# liboidcagent-go

A `go` library for `oidc-agent` is available at
https://github.com/indigo-dc/liboidcagent-go.

To use it in your `go` application include:

```go
import "github.com/indigo-dc/liboidcagent-go"
```

## Requesting an Access Token

The following functions can be used to obtain an access token for a specific account configuration or for a given OpenID
provider from `oidc-agent`.

### Token Request

All functions take a `TokenRequest` struct. This struct describes the request:

```go
type TokenRequest struct {
  // The account short name that should be used (Can be omitted if IssuerURL is
  // specified)
  ShortName string
  // The IssuerURL for which an access token should be obtained (Can be omitted
  // if ShortName is specified)
  IssuerURL string
  // MinValidPeriod specifies how long the access token should be valid at
  // least. The time is given in seconds. Default is 0.
  MinValidPeriod uint64
  // The scopes for the requested access token
  Scopes []string
  // The audiences for the requested access token
  Audiences []string
  // An string describing the requesting application (i.e. its name). It might
  // be displayed to the user, if the requested must be confirmed or an account
  // configuration loaded.
  ApplicationHint string
```

### GetAccessToken

```go
func GetAccessToken(req TokenRequest) (token string, err error)
```

This function requests an access token from oidc-agent according to the passed
[`TokenRequest`](#token-request).

#### Return Value

The function returns only the access token as a `string` and an error. To additionally obtain other information
use [`GetTokenResponse`](#gettokenresponse). On failure an error is returned.

#### Examples

##### Account Configuration Example

A complete example can look the following:

```go
token, err := liboidcagent.GetAccessToken(liboidcagent.TokenRequest{
  ShortName: accountName,
  MinValidPeriod: 60,
  Scopes: []string{"openid", "profile"},
  ApplicationHint: "Example-App",
})
if err != nil {
    var agentError *OIDCAgentError
    if errors.As(err, &agentError) {
        fmt.Printf("%s\n", agentError.ErrorWithHelp())
    }
    // Additional error handling
} else {
    fmt.Printf("Access token is: %s\n", token)
}
```

##### Issuer Example

A complete example can look the following:

```go
token, err := liboidcagent.GetAccessToken(liboidcagent.TokenRequest{
  IssuerURL: "https://openid.example.com",
  MinValidPeriod: 60,
  Scopes: []string{"openid", "profile"},
  ApplicationHint: "Example-App",
})
if err != nil {
    var agentError *OIDCAgentError
    if errors.As(err, &agentError) {
        fmt.Printf("%s\n", agentError.ErrorWithHelp())
    }
    // Additional error handling
} else {
    fmt.Printf("Access token is: %s\n", token)
}
```

### GetTokenResponse

```go
func GetTokenResponse(req TokenRequest) (resp TokenResponse, err error)
```

This function requests an access token from oidc-agent according to the passed
[`TokenRequest`](#token-request).

#### Return Value

The function returns an `TokenResponse struct` that contains the requested access token, the url of the issuer that
issued the token and the time when the token expires.

The values can be accessed the following way:

```go
response, err := liboidcagent.GetTokenResponse(...)
response.Token      // access token
response.Issuer     // issuer url
response.ExpiresAt  // expiration time
```

#### Examples

##### Account Configuration Example

A complete example can look the following:

```go
resp, err := liboidcagent.GetTokenResponse(liboidcagent.TokenRequest{
  ShortName: accountName,
  MinValidPeriod: 60,
  Audiences: []string{"https://storage.example.org"},
  ApplicationHint: "Example-App",
})
if err != nil {
  var agentError *OIDCAgentError
  if errors.As(err, &agentError) {
    fmt.Printf("%s\n", agentError.ErrorWithHelp())
  }
  // Additional error handling
} else {
  fmt.Printf("Access token is: %s\n", resp.Token)
  fmt.Printf("Issuer url is: %s\n", resp.Issuer)
  fmt.Printf("The token expires at: %s\n", resp.ExpiresAt)
}
```

##### Issuer Example

A complete example can look the following:

```go
resp, err := liboidcagent.GetTokenResponse(liboidcagent.TokenRequest{
  Issuer: "https://openid.example.com",
  MinValidPeriod: 60,
  Audiences: []string{"https://storage.example.org"},
  ApplicationHint: "Example-App",
})
if err != nil {
  var agentError *OIDCAgentError
  if errors.As(err, &agentError) {
    fmt.Printf("%s\n", agentError.ErrorWithHelp())
  }
  // Additional error handling
} else {
  fmt.Printf("Access token is: %s\n", resp.Token)
  fmt.Printf("Issuer url is: %s\n", resp.Issuer)
  fmt.Printf("The token expires at: %s\n", resp.ExpiresAt)
}
```

