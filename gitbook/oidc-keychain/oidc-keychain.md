# oidc-keychain

`oidc-keychain` enables re-using [`oidc-agent`](../oidc-agent/oidc-agent.md) between login sessions. Since
version `4.3.0` [oidc-agent-service](../oidc-agent-service/oidc-agent-service.md) is used to utilize a running agent
across sessions. Still accounts are loaded when needed (using [`oidc-add`](../oidc-add/oidc-add.md)), and the
`OIDCD_PID` and `OIDC_SOCK` environment variables are set.  `oidc-keychain` is commonly used inside `.bash_profile` or
similar to start `oidc-agent` when needed.

For example this line in `.bash_profile`

```
eval `oidc-keychain --accounts <shortname>`
```

starts `oidc-agent` when needed, loads the <shortname> account if it isn't already loaded, and sets the oidc environment
variables so other applications can locate the agent.

