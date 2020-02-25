## State of Feature Support
### What does work:
- [Installing oidc-agent using homebrew](#install-with-homebrew)
- [Building oidc-agent manually](#building-oidc-agent-manually)
- password / consent prompts that uses ssh-askpass -> requires [installing
  x11-ssh-askpass](#installing-ssh-askpass)
  - [this project](https://github.com/lukas-zronek/ssh-askpass-mac) might also
    work (not tested), but requires Xcode 9 or newer.

### What does not work:
- syslog -> we implemented a custom logging behavior. The log file can be found
in the oidc-agent directory.
  - Obviously, now oidc-agent has to write to disk, which sort of breaks
    privilege separation.
- bash completion -> chances are good to make it work with a newer version of bash (feel free to check things out and submit a pull request)
- seccomp
- storing a password in the keyring -> if one can make it work, pull requests /
  instructions are welcome.
- Xsession integration
- some enhancements might not work properly (e.g. http-server might not be
    killed in all cases when the agent dies)


