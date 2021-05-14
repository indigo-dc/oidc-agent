## Detailed Information About All Options

* [`--console`](#console)
* [`--data-storage`](#data-storage)
* [`--debug`](#debug)
* [`--json`](#json)
* [`--kill`](#kill)
* [`--log-stderr`](#log-stderr)
* [`--port`](#port)
* [`--quiet`](#quiet)

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

### `--json`
Enables json output for values like agent socket and pid. Useful when starting
the agent via scripts.

### `--kill`
This will kill the currently running agent. The agent to be killed is identified
by the `OIDCD_PID` environment variable. On a server this environment variable
most likely will not be set and the easiest way to kill (all) oidc-agent-server
is to run `killall oidc-agent-server`.

### `--log-stderr`
The `--log-stderr` option allows log messages to be printed to `stderr`.
Note that the log messages are still logged to `syslog` as usual. This option
is intended for debug purposes and is usually combined with `-d`.

### `--port`
This option is used to specify the port on which the server should start listen.
The default is `42424`. If another port is used clients have to specify it when
setting the `OIDC_REMOTE_SOCK` environment variable. It then must be of the
form `host:port`.

### `--quiet`
Silences informational messages. Currently only has effect on the generated
bash echo when setting agent environments.
