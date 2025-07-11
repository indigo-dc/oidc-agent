## `seccomp`

With version 2.0.0 we integrated `seccomp` into `oidc-agent` to restrict the system calls that each component is allowed
to perform.

**Seccomp Support was dropped again with version 4.3.0!**

Users that want to restrict `oidc-agent` further might want to have a look
at [AppArmor](https://help.ubuntu.com/lts/serverguide/apparmor.html.en).
