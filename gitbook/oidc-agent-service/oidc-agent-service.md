# oidc-agent-service

`oidc-agent-service` can be used to easily restart `oidc-agent`.
`oidc-agent-service` is called in the same way as `oidc-agent`, this means that
`oidc-agent-service` will print out the needed shell commands to set environment
variables.
Therefore `oidc-agent-service` is usually called with `eval` or its output is
piped to a file.

## Quick Start

Make `oidc-agent` available in the current terminal:

```
eval `oidc-agent-service use`
```

Restart the agent (it will still be usable in all terminals as before after the
restart):

```
oidc-agent-service restart-s
```

## Configuration

The behavior of `oidc-agent-service` can be configured through a configuration
file. Among others, this file can be used to set the command line options used
when starting the agent.
The system-wide configuration file `/etc/oidc-agent/oidc-agent-service.options`
can be adapted to change the behavior of `oidc-agent-service` for the whole system.
You can also add a `oidc-agent-service.options` file to your [oidc-agent
directory](../configuration/directory.md). Options specified in this file will
overwrite any option defined in `/etc/oidc-agent/oidc-agent-service.options`.

Please note that with oidc-agent 5 a proper configuration file was introduced; therefore, it is possible to configure
the started agent through that file. However, the above mentioned way is still supported. An overwrites options
specified in the configuration file.

## Commands

### `use`

`use` will give you an usable agent. This is usually the command you want to use
to start an agent.
It starts an agent and makes it available (it prints the needed environment
variables). If `oidc-agent-service` has already started an agent for you, this
agent will we reused and made available.

### `start`

`start` starts an agent. If `oidc-agent-service` already started an agent,
`start` will fail. If you want to reuse that agent in this case, use `use`.

### `restart`

`restart` restarts the agent. This means that the current agent is stopped and a
new agent is started. On default the new agent is started with the same options
as the old one. This behaviour can be changed (see
[configuration](#configuration)).

### `restart-s`

`restart-s` is the same as `restart`, but does not print any output. Therefore,
you can call `oidc-agent-service restart-s` instead of ``eval `oidc-agent-service
restart-s` ``.

### `stop`

`stop` stops the running agent.

### `kill`

Same as `stop`.
