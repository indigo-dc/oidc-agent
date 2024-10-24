## Installation

### Using WSL

On a modern Windows system with WSL2 (verified, WSL1 **might** work)
`oidc-agent` can be used through WSL:

- If not already done install WSL according to https://learn.microsoft.com/en-us/windows/wsl/install
- Add the `oidc-agent` repo and install according to http://repo.data.kit.edu for your chosen distribution
- Add `eval $(oidc-agent-service use)` to your `.bashrc`
- Now `oidc-agent` and friends can be used from the WSL and windows shell as
  used to, e.g.

```shell
# From WSL 
oidc-token <shortname>

# From Powershell
wsl oidc-token <shortname>
```

### "Native" Installer

We provide an installer for oidc-agent
at: [http://repo.data.kit.edu/windows/oidc-agent/](http://repo.data.kit.edu/windows/oidc-agent/).

The installer will install all necessary binaries and libraries and oidc-agent is ready to use.

We recommend to use the WSL method instead and only use this is installer if
WSL is not possible.
It is likely that support for this installer is dropped in the future.