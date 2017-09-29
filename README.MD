# oidc-agent
oidc-agent is a tool to manage OpenID Connect tokens and make them easily usable 
from the command line. We followed the ssh-agent design, so users can 
handle OIDC tokens in a similiar way as they do with ssh keys. 

oidc-agent is usually started in the beginning of an X-session or a login session. 
Through use of environment variables the agent can be located and used to handle 
oidc tokens.

The agent initially does not have any account configurations loaded.  You can load a
account configuration by using oidc-add.  Multiple accounts configurations may
be loaded in oidc-agent concurrently.  oidc-add is also used to remove a loaded
configuration from oidc-agent.

Full documentation can be found at https://indigo-dc.gitbooks.io/oidc-agent/.

## Quickstart for INDIGO IAM
Once installed a account configuration for INDIGO IAM can be generated. 

The next lines will start oidc-agent and the client registration process.
```
eval `oidc-agent`
oidc-gen <shortname>
```

A client will be registered, but it will lack permission to use the
password grant type.  Send the client-id displayed by oidc-gen to an
INDIGO IAM admin (Andrea) to update the client configuration. 

After the client configuration was updated by an admin, the account
configuration generation can be finished.  For this you need to provide
the clientconfig file generated during the previous call to oidc-gen to do
so. (Client configs are stored in <$HOME/.config/oidc-agent>.clientconfig)

```
oidc-gen -f <path_to_clientconfigfile>
```
Now the account configuration was created and already added to the agent. 

Test it with:
```
oidc-token <shortname>
```

After this initial generation the account configuration can be added to the started agent using oidc-add:
```
oidc-add <shortname>
```


## oidc-agent
You can start the agent by running:
```
oidc-agent
```
This will print out shell commands which have to be executed in the shell where
you want to run oidc-add, oidc-gen, and any client.

To start oidc-agent and directly set the needed environment variables you can use:
```
eval `oidc-agent`
```

### Persistence of oidc-agent
A simple way to make oidc-agent persistent is to include this line in your
`.bashrc`:
```
test -e ~/tmp/oidc-agent.env && . ~/tmp/oidc-agent.env
```
And to run the agent as `oidc-agent > ~/tmp/oidc-agent.env`
From now on every new shell should have access to the agent. 

You can test this with:
```
oidc-token <shortname>
```
