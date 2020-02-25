## Privilege Separation & Architecture
We followed the security by design principle and split the system’s functionalities
into multiple components. In that way we also achieved privilege separation.

The oidc-agent project consists of the following components:
- `oidc-agent`: The actual agent managing the tokens and performing all communication
with the OpenID Provider; internally also has two components:
  - oidc-agent-proxy: A proxy daemon that forwards requests to
    oidc-agent-daemon. It handles encryption passwords and file access for oidc-agent-daemon when it has to read (autoload) or write (changing refresh token) an account configuration file.
  - oidc-agent-daemon: The daemon that holds the loaded accounts and performing
    all communication with the OpenID Provider
- `oidc-gen`: A tool for generating account configuration files for usage with `oidc-agent` and
`oidc-add`.
- `oidc-add`: A tool that loads the account configurations into the agent.
- `oidc-token` and third party applications: Applications that need an OIDC access token
can obtain it through the agent’s [API](../api/api.md). One
example application for obtain access tokens is `oidc-token`.

![Architecture of the oidc-agent project](https://raw.githubusercontent.com/indigo-dc/oidc-agent/master/gitbook/images/architecture.png)
### Privileges Needed by Different Components
The following list might not be complete when it comes to implementation details
(e.g. privileges needed for obtain the current time).

Privileges for the different components:
- oidc-agent-daemon:
  - pipe ipc
  - socket ipc
  - internet
    - also reads the CA bundle file
  - starts web server
  - reads time
  - reads random
- oidc-agent-proxy:
  - creates directory in `/tmp`
  - creates socket in the created temporary directory
  - pipe ipc
  - socket ipc
  - reads time
  - reads random
  - reads files in the oidc-agent directory
  - writes files in the oidc-agent directory
- oidc-gen:
  - reads files in the oidc-agent directory
  - writes files in the oidc-agent directory
  - writes files in `/tmp`
  - writes files passed to `--output`
  - reads files passed to `--file`, `--print`
  - reads random
  - socket ipc
  - might call `xdg-open` to open the authorization url in a browser
  - might execute the user provided `--pw-cmd`
- oidc-add:
  - reads files in the oidc-agent directory
  - reads random
  - socket ipc
  - might execute the user provided `--pw-cmd`
- oidc-token:
  - socket ipc

