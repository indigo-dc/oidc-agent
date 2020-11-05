## State of Feature Support
### What does work:
- [Installing oidc-agent using homebrew](installation.md#install-with-homebrew)
- [Building oidc-agent manually](installation.md#building-oidc-agent-manually)
- password / consent prompts using `pashua` -> requires installing pashua: `brew cask install pashua`

### What does not work:
- syslog -> we implemented a custom logging behavior. The log file can be found
in the oidc-agent directory.
  - Obviously, now oidc-agent has to write to disk, which sort of breaks
    privilege separation.
- bash completion
- seccomp
- storing a password in the keyring -> if one can make it work, pull requests /
  instructions are welcome.
- Xsession integration
- some enhancements might not work properly (e.g. http-server might not be
    killed in all cases when the agent dies)
