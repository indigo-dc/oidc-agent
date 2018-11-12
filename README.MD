# oidc-agent
oidc-agent is a tool to manage OpenID Connect tokens and make them easily usable 
from the command line. We followed the ssh-agent design, so users can 
handle OIDC tokens in a similiar way as they do with ssh keys. 

oidc-agent is usually started in the beginning of an X-session or a login session. 
Through use of environment variables the agent can be located and used to handle 
oidc tokens.

The agent initially does not have any account configurations loaded.  You can load an
account configuration by using oidc-add.  Multiple accounts configurations may
be loaded in oidc-agent concurrently.  oidc-add is also used to remove a loaded
configuration from oidc-agent.

With version 2.0.0 we integrated ```seccomp``` into oidc-agent to restrict the system calls that each component is allowed to perform. However, the system calls needed may vary for different systems and architectures. Therefore, it is possible that you might face errors saying 'Bad Syscall'. If you get such an error please hand in a bug report to help improving oidc-agent further. See our [GitBook](https://indigo-dc.gitbooks.io/oidc-agent/seccomp.html) for details. You can also turn seccomp off by using the ```--no-seccomp``` option.

Full documentation can be found at https://indigo-dc.gitbooks.io/oidc-agent/.

We have a low-traffic mailing list with updates such as critical security incidents and new releases: [Subscribe oidc-agent-user](https://www.lists.kit.edu/sympa/subscribe/oidc-agent-user)

Current releases are available at [GitHub](https://github.com/indigo-dc/oidc-agent/releases) or http://cvs.fzk.de/oidc-agent
