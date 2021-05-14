## Short Information About All Options

| Option | Effect |
| -- | -- |
| [`--always-allow-idtoken`](#always-allow-idtoken) |Always allow id-token requests without manual approval by the user
| [`--confirm`](#confirm) |Requires user confirmation when an application requests an access token for any loaded
| [`--console`](#console) |Runs `oidc-agent` on the console, without daemonizing
| [`--debug`](#debug) | Sets the log level to DEBUG
| [`--json`](#json) |Print agent socket and pid as JSON instead of bash
| [`--kill`](#kill) |Kill the current agent (given by the OIDCD_PID environment variable)
| [`--no-autoload`](#no-autoload) |Disables the autoload feature: A token request cannot load the needed configuration
| [`--no-scheme`](#no-scheme) | `oidc-agent` will not use a custom uri scheme redirect [Only applies if authorization code flow is used]
| [`--no-webserver`](#no-webserver) | `oidc-agent` will not start a webserver [Only applies if authorization code flow is used]
| [`--pw-store`](#pw-store) |Keeps the encryption passwords for all loaded account configurations encrypted in memory [..]
| [`--quiet`](#quiet) |Disable informational messages to stdout
| [`--seccomp`](#seccomp) |Enables seccomp system call filtering; allows only predefined system calls
| [`--lifetime`](#lifetime) |Sets a default value in seconds for the maximum lifetime of account configurations [..]
| [`--log-stderr`](#log-stderr) |Additionally prints log messages to stderr
| [`--status`](#status) |Connects to the currently running agent and prints status information
| [`--with-group`](#with-group) |Applications running under another user can access the agent [..]

## Detailed explanation About All Options

### `--always-allow-idtoken`
`oidc-token` can also be used to request an id token from the agent. On
default such requests have to be approved by the user, since this is only meant
as a development tool and other applications should not request id tokens from
the agent as they are not meant for authorization. If the
`--always-allow-idtoken` option is specified id token requests do not need
confirmation by the user.

### `--confirm`
On default every application running as the same user as the agent can obtain an
access token for every account configuration from the agent. The `--confirm`
option can be used to change this behavior. If that option is used, the user has
to confirm each usage of an account configuration, allowing fine grained control
from the user. The `--confirm` option can be used when loading an account
configuration through `oidc-add`, in that case only that specific account needs
confirmation, or when starting the agent. If the option is used with the agent,
every usage of every account configuration has to be approved by the user.

### `--console`
Usually `oidc-agent` runs in the background as a daemon. This option will skip
the daemonizing and run on the console. This might be sued for debugging.

### `--debug`
This increases the log level to `DEBUG` and obviously should only be used to
debug purposes. If enabled, sensitive information (among others refresh tokens and client
credentials) are logged to the system log.

### `--json`
Enables json output for values like agent socket and pid. Useful when starting
the agent via scripts.

### `--kill`
This will kill the currently running agent. The agent to be killed is identified
by the `OIDCD_PID` environment variable. When integrated with Xsession this
will kill the agent available in all terminals. A restarted agent will not
automatically be available in already existing or new terminals. You can use
[`oidc-keychain`](../oidc-keychain/oidc-keychain.md) to make a newly started agent available in new terminals or login sessions.

### `--no-autoload`
On default account configurations can automatically be loaded if needed. That means
that an application can request an access token for every account configuration.
If it is not already loaded the user will be prompted to enter the needed
encryption password. After the user enters the password the account configuration
is loaded and an access token returned to the application. the user can also
cancel the autoload.

With `--no-autoload` enabled the agent will not load currently not loaded account configuration for which an access token is requested. The user then first has to add them manually by using `oidc-add`, before an application can obtain an access token for those.

### `--no-scheme
This option can be used when the authorization code flow is performed. The `--no-scheme` option tells
`oidc-agent` that a custom uri scheme should never be used for redirection
(for any account configuration). Normally a custom uri scheme can be used to
redirect direct to (another) oidc-gen instance when performing the
authorization code flow instead of using a web server. However, the redirect to
oidc-gen requires a graphical desktop environment. If this is not present,
redirection with custom uri schemes can be disabled with this option.

### `--no-webserver`
This option can be used when the authorization code flow is performed. On default a small
webserver is started by `oidc-agent` to be able to catch the redirect and
complete the authorization code flow. The `--no-webserver` option tells
`oidc-agent` that no webserver should be started (for any account
configuration). The authorization code
flow can still be completed. Either by using a redirect uri that follows the
custom redirect uri scheme `edu.kit.data.oidc-agent:/<path>` - this will
directly redirect to oidc-gen, or by copying the url the browser would normally
redirect to and pass it to `oidc-gen --codeExchange`.

### `--pw-store`
When this option is provided, the encryption password for all account
configurations  will be kept in memory by
`oidc-agent` (in an encrypted way).

This option can also be sued with `oidc-add`. When this option is used with
`oidc-agent` it applies to all loaded account configuration; when used with
`oidc-add` only for that specific one. See [`oidc-add
--pw-store`](../oidc-add/options.md#pw-store) for more information.

### `--quiet`
Silences informational messages. Currently only has effect on the generated
bash echo when setting agent environments.

### `--seccomp`
Enables seccomp system call filtering. See [general seccomp
notes](../security/seccomp.md) for more details.

### `--lifetime`
The `--lifetime` option can be used to set a default lifetime for all loaded account
configurations. This way all account configurations will only be loaded for a
limited time after which they are automatically removed from the agent.
When loading an account configuration with `oidc-add` this lifetime can be
overwritten. So that a specific account configuration can be loaded with another
lifetime (lower, higher, and also infinite).

Using `--lifetime=0` means that account configuration are not automatically
removed and they are kept loaded for an infinite time. This is also the default
behavior.

### `--log-stderr`
The `--log-stderr` option allows log messages to be printed to `stderr`.
Note that the log messages are still logged to `syslog` as usual. This option
is intended for debug purposes and is usually combined with `-d`.

### `--status`
The `--status` option can be used to obtain information about a currently
running agent. Therefore, the `OIDC_SOCK` environment variable must be set. The
option prints information such as:
- the version of the running agent (this is useful to check what agent version
    is currently running and it might differ from the version installed)
- options that can be set on start up
- the loaded accounts

### `--with-group`
On default only applications that run under the same user that also started the
agent can obtain tokens from it. The `--with-group` option can be used to also
allow other applications access to the agent. This can be useful in cases where
applications must run under a specific user. The user first has to create a
group (e.g. named `oidc-agent`) and add himself and all other users that need
access to the agent to this group. It is the user's responsibility to manage
this group. Then he can pass the group name to the `--with-group` option to
allow all group members access to the agent. If the option is used without
providing a group name, the default is `oidc-agent`.
