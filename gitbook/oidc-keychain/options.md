## Detailed Information About All Options

* [`--accounts`](#accounts)
* [`--kill`](#kill)

### `--accounts`
Makes sure that the given ACCOUNT short names are loaded by calling oidc-add
when they aren't already loaded.

### `--kill`
Kills any running oidc-agent, whether or not the environment variables
are loaded in the current shell.  When used in backquotes, this will
unset the `OIDCD_PID` and `OIDC_SOCK` environment variables in
the current shell.
