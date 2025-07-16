## oidc-agent Integration
### Xsession Integration
oidc-agent has support for integration with Xsession, which is enabled by
default if the `oidc-agent-desktop` package is installed. This means oidc-agent will automatically start at the beginning of an 
Xsession and then be available throughout that session (i.e. you can connect to
the agent from every terminal).

To disable / re-enable this behavior (system-wide) edit the `/etc/oidc-agent/oidc-agent-service.options` file. To disable it on a per user basis, copy this file to your [oidc-agent directory](directory.md) and edit it there.
To disable XSession integration uncomment / add the line:
```
START_AGENT_WITH_XSESSION="False"
```
To re-enable Xsession integration change it to `START_AGENT_WITH_XSESSION="True"` or comment it out.

To pass command line options to the automatically started agent edit the
`OIDC_AGENT_OPTS` variable.

Note that from version 4.1.0 on the agent can be restarted without losing the
integration in existing terminals. To do so run:
```
oidc-agent-service restart-s
```

We also want to note that
[`oidc-agent-service`](../usage/oidc-agent-service/index.md) can be used without Xsession
integration.
