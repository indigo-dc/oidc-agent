## Privilege Separation & Architecture

We followed the security by design principle and split the system’s functionalities into multiple components. In that
way we also achieved privilege separation.

The oidc-agent project consists of the following components:

- `oidc-agent`: The actual agent managing the tokens and performing all communication with the OpenID Provider;
  internally also has two components:
    - oidc-agent-proxy: A proxy daemon that forwards requests to oidc-agent-daemon. It handles encryption passwords and
      file access for oidc-agent-daemon when it has to read (autoload) or write (changing refresh token) an account
      configuration file.
    - oidc-agent-daemon: The daemon that holds the loaded accounts and performing all communication with the OpenID
      Provider
- `oidc-gen`: A tool for generating account configuration files for usage with `oidc-agent` and
  `oidc-add`.
- `oidc-add`: A tool that loads the account configurations into the agent.
- `oidc-token` and third party applications: Applications that need an OIDC access token can obtain it through the
  agent’s [API](../api/index.md). One example application for obtain access tokens is `oidc-token`.

![Architecture of the oidc-agent project](https://raw.githubusercontent.com/indigo-dc/oidc-agent/master/gitbook/images/architecture.png)
