## Detailed Information About All Options

* [`--console`](#console)
* [`--data-storage`](#data-storage)
* [`--debug`](#debug)
* [`--kill`](#kill)
* [`--log-stderr`](#log-stderr)
* [`--port`](#port)
### `--console`
Usually oidc-agent runs in the background as a daemon. This option will skip
the daemonizing and run on the console. This might be sued for debugging.

### `--data-storage`
This option is used to specify the location where the loaded accounts are
stored. With the central oidc-agent loaded accounts are not kept in memory but
encrypted on disk. The encrypted account configuration files are stored in the
specified location. On default it is `/tmp/oidc-agent-server`.

### `--debug`
This increases the log level to `DEBUG` and obviously should only be used to
debug purposes. If enabled, sensitive information (among others refresh tokens and client
credentials) are logged to the system log.

### `--kill`
This will kill the currently running agent. The agent to be killed is identified
by the `OIDCD_PID` environment variable. When integrated with Xsession this
will kill the agent available in all terminals. A restarted agent will not
automatically be available in already existing or new terminals. You can use
[`oidc-keychain`](../oidc-keychain/oidc-keychain.md) to make a newly started agent available in new terminals or login sessions.

### `--log-stderr`
The `--log-stderr` option allows log messages to be printed to `stderr`.
Note that the log messages are still logged to `syslog` as usual. This option
is intended for debug purposes and is usually combined with `-d`.

### `--port`
This option is used to specify the port on which the server should start listen.
The default is `42424`. If another port is used clients have to specify it when
setting the `OIDC_REMOTE_SOCK` environment variable. It then must be of the
form `host:port`.
