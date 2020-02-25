## Starting oidc-agent
As described in [Xsession
integration](../configuration/integration.md#xsession-integration), by
default oidc-agent is integrated with Xsession. Therefore, it is automatically
started and available in all terminals through that session. So usually a user
does not have to start oidc-agent. 

After installing oidc-agent the agent will not be automatically available. After
a system restart the agent can be used in all terminals.

The agent can also be started by using:
```
oidc-agent
```
This will print out shell commands which have to be executed in the shell where
you want to run oidc-add, oidc-gen, and any application using oidc-agent.

To start oidc-agent and directly set the needed environment variables you can use:
```
eval `oidc-agent`
```


