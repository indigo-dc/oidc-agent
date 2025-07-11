## Credentials

(User) credentials are very sensitive information and have to be handled with
adequate caution.

### User Credentials

The user only has to pass its credentials (for the OpenID Provider) to
`oidc-agent` when using the password flow. This flow has to be explicitly enabled
to use it with `oidc-gen`. Furthermore, it is not even supported by most
providers and if, it might require manual approval from an OpenID Provider
administrator. It is recommended to use one of the other flows.
However, when user credentials are passed to `oidc-agent` we handle them
carefully and keep them as short as possible in memory. Credentials are also
overwritten before the memory is freed (see also [memory](memory.md)) and on disk only stored in the encrypted account
configuration.

### Refresh Tokens

OpenID Connect refresh tokens can be used to obtain additional access
tokens and must be kept secret. The refresh token is stored encrypted in
the account configuration file (s. [account configuration
files](account-configs.md)). The refresh token is only read by `oidc-gen`
(during account configuration generation) and `oidc-add` (when adding a
configuration to the agent). When using the autoload feature (see [account
autoload](../tips.md#autoloading-and-autounloading-account-configurations))
also `oidc-agent` reads the refresh token from the encrypted account
configuration file. However, `oidc-gen` and `oidc-add` do not use the
refresh token, they only pass it to `oidc-agent`. `oidc-agent` uses the
refresh token to obtain additional access tokens.

**The agent has to keep the refresh token in memory. However, when it is not used
it will be obfuscated, so it is harder to extract it from a memory dump.
The password used for this obfuscation is dynamically generated when the agent
starts.**

**The refresh token cannot be requested from `oidc-agent`and is never send to any other application.**


