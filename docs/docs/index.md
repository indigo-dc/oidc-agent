![oidc-agent logo](https://raw.githubusercontent.com/indigo-dc/oidc-agent/master/logo_wide.png)
# oidc-agent
oidc-agent is a set of tools to manage OpenID Connect tokens and make them easily usable 
from the command line. We followed the
[`ssh-agent`](https://www.openssh.com/) design, so users can 
handle OIDC tokens in a similar way as they do with ssh keys. 

`oidc-agent` is usually started in the beginning of an X-session or a login session. 
Through use of environment variables the agent can be located and used to handle 
OIDC tokens.

The agent initially does not have any account configurations loaded.  You can load an
account configuration by using `oidc-add`.  Multiple account configurations may
be loaded in `oidc-agent` concurrently.  `oidc-add` is also used to remove a loaded
configuration from `oidc-agent`. `oidc-gen` is used to initially generate an account
configurations file [(Help for different providers)](provider/index.md).

We have a low-traffic **mailing list** with updates such as critical security incidents and new releases: [Subscribe oidc-agent-user](https://www.lists.kit.edu/sympa/subscribe/oidc-agent-user)

Current releases are available at [GitHub](https://github.com/indigo-dc/oidc-agent/releases) or http://repo.data.kit.edu/
