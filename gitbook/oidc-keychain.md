# oidc-keychain

oidc-keychain enables re-using [```oidc-agent```](oidc-agent.md) between
login sessions.  It stores oidc-agent environment variables in a file
and takes care of starting oidc-agent when needed, loading any given
accounts when needed (using [```oidc-add```](oidc-add.md)), and setting the
```OIDCD_PID``` and ```OIDC_SOCK``` environment variables.  It is
commonly used inside ```.bash_profile``` or similar to start oidc-agent
when needed.

For example this line in ```.bash_profile```
```
eval `oidc-keychain --accounts <shortname>`
```
will start oidc-agent when needed, load the <shortname> account if
it isn't already loaded, and set the oidc environment variables.

## General Usage
```
Usage: oidc-keychain [-?|--help|--usage|-V|--version] [-k|--kill]
   or: oidc-keychain [oidc-agent options] [--accounts ACCOUNT ...]
```

Any given oidc-agent options will get passed to oidc-agent when it needs
to be started.

See [Detailed Information About All
Options](#detailed-information-about-all-options) for more information.

## Detailed Information About All Options

* [`--accounts`](#-accounts)
* [`--kill`](#-kill)

### `--accounts`
Makes sure that the given ACCOUNT short names are loaded by calling oidc-add
when they aren't already loaded.

### `--kill`
Kills any running oidc-agent, whether or not the environment variables
are loaded in the current shell.  When used in backquotes, this will
unset the ```OIDCD_PID``` and ```OIDC_SOCK``` environment variables in
the current shell.
