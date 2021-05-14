## Detailed Information About All Options

* [`--loaded`](#loaded)
* [`--always-allow-idtoken`](#always-allow-idtoken)
* [`--confirm`](#confirm)
* [`--list`](#list)
* [`--print`](#print)
* [`--pw-cmd`](#pw-cmd)
* [`--pw-env`](#pw-env)
* [`--pw-file`](#pw-file)
* [`--pw-keyring`](#pw-keyring)
* [`--pw-prompt`](#pw-prompt)
* [`--pw-store`](#pw-store)
* [`--remove`](#remove)
* [`--remote`](#remote)
* [`--remove-all`](#remove-all)
* [`--seccomp`](#seccomp)
* [`--lifetime`](#lifetime)
* [`--lock`](#lock)
* [`--unlock`](#unlock)

### `--loaded`
This option is used without a shortname, because it will not load an account
configuration. Using this option `oidc-add` will print out a list of all
the account configurations currently loaded in the agent.

### `--always-allow-idtoken`
`oidc-token` can also be used to request an id token from the agent. On
default such requests have to be approved by the user, since this is only meant
as a development tool and other applications should not request id tokens from
the agent as id tokens are not meant for authorization. If the
`--always-allow-idtoken` option is specified to `oidc-add` when an account
configuration is loaded, id token requests for that account do not need
confirmation by the user.

### `--confirm`
On default every application running as the same user as the agent can obtain an
access token for every account configuration from the agent. The `--confirm`
option can be used to change this behavior. If that option is used, the user has
to confirm each usage of an account configuration, allowing fine grained control
from the user. When using this option with `oidc-add` only that specific account needs
confirmation.

### `--list`
This option is used without a shortname, because it will not load an account
configuration. Using this option `oidc-add` will print out a list of all
configured account configurations. Configured means that they are saved on the
system and can be loaded with `oidc-add`; it does not mean that they are
currently loaded. To show a list of currently loaded accounts, use
`--loaded`.

### `--print`
Instead of loading the account configuration with the specified shortname, it
will decrypt and print this configuration.

### `--pw-cmd`
The argument passed has to be a command that prints the encryption password for
that account configuration to `stdout` when executed. E.g. such a command could
be `echo "superSecretPassword"`. (Note that this command is not recommended, because the password is logged to your bash history.)

The command is used by `oidc-add` to retrieve the encryption password, so
the user will not be prompted for it. Additionally, it will also be used by
`oidc-agent` to get the encryption password when it needs to update the
account configuration (see [`--pw-store`](#pw-store) for information on
why `oidc-agent` might need the encryption password).

See [Encryption Passwords](../security/encryption-passwords.md) for security
related information about the different `--pw-*` options.

### `--pw-env`
By default `oidc-add` will prompt the user for an encryption password when
it needs to decrypt an account configuration.
The option `--pw-env` can be used to provide the encryption password via an
environment variable. The name of the environment variable can be passed to
`--pw-env`. If this option is used without an argument the
encryption password is read from the environment variable `OIDC_ENCRYPTION_PW`.

### `--pw-file`
The argument passed has to be the path to a file that contains the encryption
password.

The password-file is used by `oidc-add` to retrieve the encryption password, so
the user will not be prompted for it. Additionally, it will also be used by
`oidc-agent` to get the encryption password when it needs to update the
account configuration (see [`--pw-store`](#pw-store) for information on
why `oidc-agent` might need the encryption password).

See [Encryption Passwords](../security/encryption-passwords.md) for security
related information about the different `--pw-*` options.

### `--pw-keyring`
When this option is provided, the encryption password will be stored by
`oidc-agent` in the system's default keyring. See [`--pw-store`](#pw-store) for information on
why `oidc-agent` might need the encryption password.

See [Encryption Passwords](../security/encryption-passwords.md) for security
related information about the different `--pw-*` options.

### `--pw-prompt`
This option can be used to change how `oidc-add` prompts the user for the
encryption password. Possible values are `cli` and `gui`. The default is `cli`.
`gui` requires oidc-agent-prompt to be installed.

### `--pw-store`
When this option is provided, the encryption password will be kept in memory by
`oidc-agent` (in an encrypted way).
Usually none of the `--pw-*` options is needed, because `oidc-agent` does not
have to read or update the account configuration file after loading. However,
some OpenID Providers might use rotating refresh tokens. This means that for
those providers `oidc-agent` has to update the client configuration file whenever
a new access token is retrieved from the OpenID Provider. If non of the
`--pw-*` options is provided, this means that the user will always be prompted
to enter the encryption password. Because this can get annoying, it is
recommended to use any of the `--pw-*` options in such a case. For providers
that are effected by this we included notes in the [Help for different providers](../provider/provider.md).

See [Encryption Passwords](../security/encryption-passwords.md) for security
related information about the different `--pw-*` options.

### `--remove`
The `--remove` option is used to unload an account configuration. After
unloading an account, it is not longer available for other applications.
Therefore, it has to be loaded again before an access token can be obtained
(either using oidc-add or through the autoload feature).

### `--remote`
This option is used to communicate with a remote `oidc-agent-server` instead of
a local `oidc-agent`. It can only be used for loading and unloading
configurations. For more information refer to
[`oidc-agent-server`](oidc-agent-server/oidc-agent-server.md)

### `--remove-all`
 With the `--remove-all` option all loaded account configuration can be removed from the agent
with just one call. This might be preferred over restarting the agent, because
that way the agent will still be available everywhere.

### `--seccomp`
Enables seccomp system call filtering. See [general seccomp
notes](../security/seccomp.md) for more details.

### `--lifetime`
The `--lifetime` option can be used to set a lifetime for the loaded account
configuration. This way the account configuration will only be loaded for a
limited time after which it is automatically removed from the agent.
If a default lifetime was specified when the agent was started, the
`oidc-add` option has priority and can overwrite the default lifetime for
this account.

Using `--lifetime=0` means that the account configuration is not automatically
removed. Because that's the default behavior this option is only needed, if
another default lifetime was specified with oidc-agent.

### `--lock`
The agent can be locked using the `--lock` option. While being locked the agent
refuses all requests. This means that no account configuration can be loaded /
unloaded and no token can be obtained from the agent.
Sensitive information will be encrypted when the agent is locked (see also
[Memory Encryption](../security/memory.md)).

### `--unlock`
To unlock a locked agent the `--unlock` option is used. After unlocking the
agent again accepts requests.
