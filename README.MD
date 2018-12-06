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

Full documentation can be found at https://indigo-dc.gitbooks.io/oidc-agent/.

We have a low-traffic mailing list with updates such as critical security incidents and new releases: [Subscribe oidc-agent-user](https://www.lists.kit.edu/sympa/subscribe/oidc-agent-user)

Current releases are available at [GitHub](https://github.com/indigo-dc/oidc-agent/releases) or http://repo.data.kit.edu/

## Installation
### Debian Packages
- `sudo apt-key adv --keyserver hkp://hkps.pool.sks-keyservers.net --recv-keys ACDFB08FDC962044D87FF00B512839863D487A87`

- Depending on your distribution, choose one of the following lines:
     ``` 
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/stable ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/stretch ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/testing ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/buster ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/16.04 ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/xenial ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/18.04 ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/bionic ./"
     ```

- `sudo apt-get update`

- `sudo apt-get install oidc-agent`

### From Source
Rever to the [documentation](https://indigo-dc.gitbooks.io/oidc-agent/admin.html)
