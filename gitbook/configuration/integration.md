## oidc-agent Integration
### Xsession Integration
oidc-agent has support for integration with Xsession, which is enabled by
default. This means oidc-agent will automatically start at the beginning of an 
Xsession and then be available throughout that session (i.e. you can connect to
the agent from every terminal).

To disable / re-enable this behavior edit the `/etc/X11/Xsession.options` file. If the line `use-oidc-agent` is present oidc-agent will automatically be started at the beginning of an Xsession.

To pass command line options to the automatically started agent edit the
`OIDCAGENTARGS` variable in the file `/etc/X11/Xsession.d/91oidc-agent`.

[`oidc-keychain`](../oidc-keychain/oidc-keychain.md) is a simple alternative to Xsession integration.


