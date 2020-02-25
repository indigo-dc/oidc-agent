# oidc-keychain

oidc-keychain enables re-using [`oidc-agent`](../oidc-agent/oidc-agent.md) between
login sessions.  It stores oidc-agent environment variables in a file
and takes care of starting oidc-agent when needed, loading any given
accounts when needed (using [`oidc-add`](../oidc-add/oidc-add.md)), and setting the
`OIDCD_PID` and `OIDC_SOCK` environment variables.  It is
commonly used inside `.bash_profile` or similar to start oidc-agent
when needed.

For example this line in `.bash_profile`
```
eval `oidc-keychain --accounts <shortname>`
```
will start oidc-agent when needed, load the <shortname> account if
it isn't already loaded, and set the oidc environment variables.

