![oidc-agent logo](https://raw.githubusercontent.com/indigo-dc/oidc-agent/master/logo_wide.png)
<!-- [![Build Status](https://jenkins.indigo-datacloud.eu/buildStatus/icon?job=Pipeline-as-code/oidc-agent/master)](https://jenkins.indigo-datacloud.eu/job/Pipeline-as-code/job/oidc-agent/job/master/) -->
[![License](https://img.shields.io/github/license/indigo-dc/oidc-agent.svg)](https://github.com/indigo-dc/oidc-agent/blob/master/LICENSE)
[![Code size](https://img.shields.io/github/languages/code-size/indigo-dc/oidc-agent.svg)](https://github.com/indigo-dc/oidc-agent/tree/master/src)
[![Release date](https://img.shields.io/github/release-date/indigo-dc/oidc-agent.svg)](https://github.com/indigo-dc/oidc-agent/releases/latest)
[![Release version](https://img.shields.io/github/release/indigo-dc/oidc-agent.svg)](https://github.com/indigo-dc/oidc-agent/releases/latest)
<!-- [![Commits since latest release](https://img.shields.io/github/commits-since/indigo-dc/oidc-agent/latest.svg)](https://github.com/indigo-dc/oidc-agent/compare/latest...master) -->
<!-- [![Commit activity](https://img.shields.io/github/commit-activity/m/indigo-dc/oidc-agent.svg)](https://github.com/indigo-dc/oidc-agent/graphs/commit-activity) -->
<!-- [![Github downloads](https://img.shields.io/github/downloads/indigo-dc/oidc-agent/total.svg?label=github%20downloads&logo=github&style=flat)](https://github.com/indigo-dc/oidc-agent/releases) -->

# oidc-agent

oidc-agent is a set of tools to manage OpenID Connect tokens and make them easily usable from the command line. We
followed the
[`ssh-agent`](https://www.openssh.com/) design, so users can handle OIDC tokens in a similar way as they do with ssh
keys.

`oidc-agent` is usually started in the beginning of an X-session or a login session. Through use of environment
variables the agent can be located and used to handle OIDC tokens.

The agent initially does not have any account configurations loaded. You can load an account configuration by
using `oidc-add`. Multiple account configurations may be loaded in `oidc-agent` concurrently.  `oidc-add` is also used
to remove a loaded configuration from `oidc-agent`. `oidc-gen` is used to initially generate an account configurations
file [(Help for different providers)](https://indigo-dc.gitbook.io/oidc-agent/user/oidc-gen/provider).

**Full documentation** can be found at https://indigo-dc.github.io/oidc-agent/.

We have a low-traffic **mailing list** with updates such as critical security incidents and new
releases: [Subscribe oidc-agent-user](https://www.lists.kit.edu/sympa/subscribe/oidc-agent-user)

## Installation

oidc-agent is directly available for some distributions.
Additionally, we build the newest packages fora wide range of different
distributions that are available at: http://repo.data.kit.edu/

### Linux

#### Debian 12 and newer / Ubuntu 22.04 and newer

```shell
sudo apt-get install oidc-agent
```

#### Other distributions

See http://repo.data.kit.edu/

### MacOS

```
brew tap indigo-dc/oidc-agent
brew install oidc-agent
```

### windows

The installer for windows is available at http://repo.data.kit.edu/windows/oidc-agent

### From Source

Refer to the [documentation](https://indigo-dc.github.io/oidc-agent/installation/install/#from-source)

### Quickstart

After [installation](https://indigo-dc.gitbook.io/oidc-agent/installation/install) the agent has to be started. Usually
the agent is started on system startup and is then available on all terminals (
see [integration](https://indigo-dc.gitbook.io/oidc-agent/configuration/integration)). Therefore, after installation the
options are to restart your X-Session or to start the agent manually.

```
eval `oidc-agent-service start`
```

This starts the agent and sets the required environment variables.

#### Create an agent account configuration with oidc-gen

For most OpenID Connect providers an agent account configuration can be created with one of the following calls. Make
sure that you can run a web-browser on the same host where you run the `oidc-gen` command.

```
oidc-gen <shortname>
oidc-gen --pub <shortname>
```

For more information on the different providers refer
to [integrate with different providers](https://indigo-dc.gitbook.io/oidc-agent/user/oidc-gen/provider).

**`oidc-gen` supports different OIDC flows. To use the device flow instead of the authorization code flow include
the `--flow=device` option.**

After an account configuration is created it can be used with the shortname to obtain access tokens. One does not need
to run `oidc-gen` again unless to update or create a new account configuration.

#### Use oidc-add to load an account configuration

```
oidc-add <shortname>
```

However, usually it is not necessary to load an account configuration with
`oidc-add`. One can directly request an access token for a configuration and
`oidc-agent` will automatically load it if it is not already loaded.

#### Obtaining an access token

```
oidc-token <shortname>
```

Alternatively, it is also possible to request an access token without specifying the shortname of a configuration but
with the issuer url:

```
oidc-token <issuer_url>
```

This way is recommended when writing scripts that utilize oidc-agent to obtain access tokens. This allows that the
script can be easily used by others without them having to update the shortname.

#### List existing configuration

```
oidc-add -l
oidc-gen -l
```

These commands both give a list of all existing account configurations.

A list of the currently loaded accounts can be retrieved with:

```
oidc-add -a
```

#### Updating an existing account configuration

An existing account configuration can be updated with `oidc-gen`:

```
oidc-gen -m <shortname>
```

#### Reauthenticating

If the refresh token stored in the account configuration expired a new one must be created. However, it is not required
to create a new account configuration, it is enough to run:

```
oidc-gen <shortname> --reauthenticate
```

## Usage with SSH

`oidc-agent` supports your work on remote hosts in two ways:

### Create an agent account configuration on a remote host

On remote hosts you usually have no way to start a web browser for authentication. In such scenarios, the **device
flow** can be used, but adding the `flow=device` option to `oidc-gen`:

```
oidc-gen --flow=device<shortname>
```

### Agent Forwarding

To use on oidc-agent on one host (typically your workstation or laptop)
from ssh-logins to other a remote host, you need to forward the local socket of `oidc-agent` to the remote side, and
there point the `OIDC_SOCK`
environment variable to the forwarded socket. Details for what we call
"agent-forwarding", are
described [here in the gitbook](https://indigo-dc.gitbook.io/oidc-agent/configuration/forwarding).

