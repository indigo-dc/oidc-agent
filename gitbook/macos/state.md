## State of Feature Support

### What does work:

- [Installing oidc-agent using homebrew](installation.md#install-with-homebrew)
- [Building oidc-agent manually](installation.md#building-oidc-agent-manually)
- Every basic functionality including password / consent prompts

### What does not work:

- syslog -> we implemented a custom logging behavior. The log file can be found in the oidc-agent directory.
    - Obviously, now oidc-agent has to write to disk, which sort of breaks privilege separation.
- bash completion
- Xsession integration
- some enhancements might not work properly (e.g. http-server might not be killed in all cases when the agent dies)
