# oidc-agent-server

oidc-agent-server is a special version of oidc-agent that can run as a central oidc-agent on a server.
Clients can connect to this agent from other machines. In this way oidc-agent becomes available for multiple machines.
Exactly like the normal agent `oidc-agent-server` manages all OpenID
Connect communication with the OpenID Providers. Compared to the normal
`oidc-agent` it has improved separation of the loaded accounts, so it is better
suited when one agent is used for multiple users.

While communication between clients and the server is encrypted, it can be
intercepted. Man-in-the-middle attacks as well as replay attacks are possible.
If you want to run `oidc-agent-server` you should be aware of this and check if
it is a problem for your use case.

`oidc-agent-server` is a temporary component. We plan to release a new service
called `my-token` in spring 2021. The `my-token` will be more powerful and
secure. This will deprecate `oidc-agent-server`.

## Supported operations
`oidc-agent-server` only supports the following operations:
- loading and unloading an account configuration with `oidc-add`
- obtaining an access token

All other operations usually supported are not allowed. Especially it is not
supported to automatically load an account when requesting an access token and
to create a new account configuration. Only already existing account
configurations can be used with `oidc-agent-server`.

## How it works
To use a central `oidc-agent-server` a locally already existing account
configuration can be loaded and then access tokens can be obtained from any
machine by using the new short name.

For all communication with the `oidc-agent-server` the `OIDC_REMOTE_SOCK`
environment variable must be set. The value must be of the form `host[:port]`.
`host` can be a hostname resolvable by the machine or an ip address. If the port
is omitted the default value is `42424`.

### Loading an Account Configuration
To load an account configuration use:
```
oidc-add --remote <shortname>
```
where `<shortname>` is the shortname of an account configuration that exists
locally.

The account configuration is then locally decrypted and transferred to the
remote `oidc-agent-server`. Communication between the remote agent and clients
is encrypted (however, please check the note at the top). The agent receives the account configuration, generates a new random 14 characters long password which is used to encrypt the account configuration, and stores the encrypted configuration under a new random
6 characters long internal shortname. 
The local `oidc-add` receives a new shortname that encodes the encryption
password as well as the server's internal shortname. This new shortname must be used to obtain access tokens from
`oidc-agent-server` and to unload the configuration.

It is important to store
this new shortname to be able to obtain access tokens from the central agent. 
It is also important to keep this new shortname confidential since it allows
obtaining tokens without any further authentication.

### Obtaining Access Tokens
The obtained new shortname can be used with `oidc-token` or any other agent
client that uses on of the provided libraries to obtain access tokens from
`oidc-agent-server`.
When the `OIDC_REMOTE_SOCK` environment variable is set correctly it is enough
to call `oidc-token <new_shortname>` as usual. It is also possible to use any of
the `oidc-token` options.

When receiving a token request the `oidc-agent-server` uses the shortname with
the encoded password to decrypt the account configuration and load it into the
agent. Then the access token is retrieved from the provider and the account
configuration is unloaded before the token is returned to the client.

This way there will never be an account configuration loaded so that other could
steel an access token. 
This also means that a client always receives a fresh access token.

### Unloading an Account Configuration
To unload an account configuration the new shortname must be used:
```
oidc-add --remote -r <new_shortname>
```
As usual the account is no longer available after unloading.

When receiving an unload request `oidc-agent-server` checks if the shortname has
the correct password for that account encoded and if correct, deletes the configuration
file.
