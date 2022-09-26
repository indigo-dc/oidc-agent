# liboidcagent-go

A `go` library for `oidc-agent` is available at
https://github.com/indigo-dc/liboidcagent-go.

To use it in your `go` application do

```shell
go get github.com/indigo-dc/liboidc-agent-go
```

and include:

```go
import "github.com/indigo-dc/liboidcagent-go"
```

## Requesting an Access Token

The following functions can be used to obtain an access token for a specific account configuration or for a given OpenID
provider from `oidc-agent`.

### Token Request

All functions take a `TokenRequest` struct. This struct describes the request:

```go
// TokenRequest is used to request an access token from the agent
type TokenRequest struct {
	// ShortName that should be used (Can be omitted if IssuerURL is specified)
	ShortName string
	// IssuerURL for which an access token should be obtained (Can be omitted
	// if ShortName is specified)
	IssuerURL string
	// MinValidPeriod specifies how long the access token should be valid at
	// least. The time is given in seconds. Default is 0.
	MinValidPeriod uint64
	// The scopes for the requested access token
	Scopes []string
	// The audiences for the requested access token
	Audiences []string
	// A string describing the requesting application (i.e. its name). It might
	// be displayed to the user, if the request must be confirmed or an account
	// configuration loaded.
	ApplicationHint string
}
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

## Requesting a Mytoken

The following functions can be used to obtain a mytoken from `oidc-agent`.

### Mytoken Request

All functions take a `MytokenRequest` struct. This struct describes the request:

```go
// MytokenRequest is used to request a  mytoken from the agent
type MytokenRequest struct {
	// ShortName that should be used
	ShortName string
	// A mytoken profile describing the properties of the requested mytoken
	// https://mytoken-docs.data.kit.edu/concepts/profiles/
	MytokenProfile string
	// A string describing the requesting application (i.e. its name). It might
	// be displayed to the user, if the request must be confirmed or an account
	// configuration loaded.
	ApplicationHint string
}
```

### GetMytoken

```go
func GetMytoken(req MytokenRequest) (token string, err error)
```

This function requests a mytoken from oidc-agent according to the passed
[`MytokenRequest`](#mytoken-request).

#### Return Value

The function returns only the mytoken as a `string` and an error. To additionally obtain other information
use [`GetMytokenResponse`](#getmytokenresponse). On failure an error is returned.

#### Example

A complete example can look the following:

```go
token, err := liboidcagent.GetMytoken(liboidcagent.MytokenRequest{
  ShortName: accountName,
  MytokenProfile: profile,
  ApplicationHint: "Example-App",
})
if err != nil {
    var agentError *OIDCAgentError
    if errors.As(err, &agentError) {
        fmt.Printf("%s\n", agentError.ErrorWithHelp())
    }
    // Additional error handling
} else {
    fmt.Printf("Mytoken is: %s\n", token)
}
```

### GetMytokenResponse

```go
func GetMytokenResponse(req MytokenRequest) (resp MytokenResponse, err error)
```

This function requests a mytoken from oidc-agent according to the passed
[`MytokenRequest`](#mytoken-request).

#### Return Value

The function returns a `MytokenResponse struct` that contains the requested token, as well as other information.

The values can be accessed the following way:

```go
response, err := liboidcagent.GetMytokenResponse(...)
response.Mytoken        // mytoken (or transfer code)
response.MytokenType    // mytoken type
response.OIDCIssuer     // OP issuer url
response.MytokenIssuer  // mytoken issuer url
response.ExpiresAt      // expiration time
```

There are additional fields available from
the [mytoken API's `MytokenResponse`](https://pkg.go.dev/github.com/oidc-mytoken/api) struct which is included in this
libraries `MytokenResponse` struct.
<!-- TODO more detailed link -->

#### Example

A complete example can look the following:

```go
resp, err := liboidcagent.GetMytokenResponse(liboidcagent.MytokenRequest{
  ShortName: accountName,
  MytokenProfile: profile,
  ApplicationHint: "Example-App",
})
if err != nil {
  var agentError *OIDCAgentError
  if errors.As(err, &agentError) {
    fmt.Printf("%s\n", agentError.ErrorWithHelp())
  }
  // Additional error handling
} else {
  fmt.Printf("Mytoken is: %s\n", resp.Mytoken)
  fmt.Printf("Mytoken issued by: %s\n", resp.MytokenIssuer)
  fmt.Printf("Obtains ATs for: %s\n", resp.OIDCIssuer)
  fmt.Printf("The token expires at: %s\n", resp.ExpiresAt)
}
```

## Getting Loaded Accounts

```go
func GetLoadedAccounts() (accountNames []string, err error) {
```

This function requests the list of currently loaded accounts from oidc-agent.

#### Return Value

The function returns a list of the currently loaded accounts as a `[]string` on success and an `OIDCAgentError` on
failure.

## Getting Configured Accounts

```go
func GetConfiguredAccounts() (accounts []string) {
```

This function checks the oidc-agent directory for the configured accounts.

#### Return Value

The function returns a list of the configured accounts as a `[]string`.
