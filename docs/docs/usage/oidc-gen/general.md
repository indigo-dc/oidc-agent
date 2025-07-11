## General Usage
Usually `oidc-gen` is used in one of two ways: Using dynamic client
registration (default) or using an already registered client (`-m`).
For providers that support dynamic client registration a simple call to
`oidc-gen` is enough. You can also directly provide the shortname of the new
account configuration: `oidc-gen <shortname>`
After a successful account configuration generation oidc-gen will save the
encrypted account configuration file in the [oidc-agent directory](../../configuration/directory.md) using the
shortname as the filename.

```
Usage: oidc-gen [OPTION...] [ACCOUNT_SHORTNAME]
```

Internal options are not considered part of the public API, even if listed for
completeness. They can change at any time without backward compatibility
considerations.

See [Detailed Information About All
Options](options.md) for more information.

### Client Registration
`oidc-agent` requires a registered client for every OpenID Provider used. Most likely a user
does not have an already registered client and does not want to do it through a web interface.
If the OpenID Provider supports dynamic client registration, the agent can register a new client dynamically.
One big advantage of using dynamic registration is the fact that oidc-agent will
register the client with exactly the configuration it needs.
Dynamic Registration is the default option and running `oidc-gen` is enough.

If a user already has a client registered or the OpenID Provider does not support
dynamic client registration `oidc-gen` must be called with the `-m` option. `oidc-gen` will prompt the user for the relevant
information. If the user has a file with the client configuration information they can pass it to oidc-gen using the `-f` flag.
When registering a client manually be careful with the provided data. Check
[Client Configuration Values](../../provider/client-configuration-values.md) for the values that are important to oidc-agent.

See [Provider Info](../../provider/index.md) on how to generate an account configuration for a specific
provider.

### oidc-gen and oidc-add
`oidc-gen` will also add the generated configuration to the agent. So you don't
have to run `oidc-add` afterwards. However, if you want to load an existing
configuration don't use `oidc-gen` for it; [`oidc-add`](../oidc-add/index.md) is your friend.

### Edit an existing account configuration
To edit an existing configuration, call `oidc-gen -m <shortname>` where `<shortname>` is the short name for that configuration.

If you only have to update the refresh token and do not want to change any other
data for this account configuration, use `oidc-gen --reauthenticate <shortname>`.

