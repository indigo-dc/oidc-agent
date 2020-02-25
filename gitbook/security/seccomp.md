## `seccomp`
With version 2.0.0 we integrated `seccomp` into `oidc-agent` to restrict the system calls that each component is allowed to perform. 
However, the system calls needed may vary for different operating system (versions) and architectures. Therefore, we decided to disable this feature by default.
(expert) users with higher security standards can turn seccomp on by using the `--seccomp` option. (It does make sense to define aliases.) However, these users will have to maintain the white-listed system calls on their own. The configuration files are located in `/etc/oidc-agent/privileges/`
Because seccomp can not restrict string parameters (e.g. paths) the added security is rather small. This is because even though privilege separation is in place and e.g. `oidc-agent` does not have to read and write regular files, it still needs system calls that can be sued to read and write regular files, because `oidc-agent` also needs to create a tmp directory, create sockets, read and write from sockets and pipes, read time and random, ...

Users that want to restrict `oidc-agent` further might want to have a look
at [AppArmor](https://help.ubuntu.com/lts/serverguide/apparmor.html.en).

### Missing Seccomp System Calls

A missing system call can be very hard to debug. The first step is to ensure
that there really is a missing system call. For all client components (`oidc-add`,
`oidc-gen`, etc.) this is very easy. If they are killed by the kernel you probably
will see a `Bad Syscall` message on your terminal. However, for `oidc-agent` this
message will not appear so it's not that easy to tell if `oidc-agent` was killed
because of a seccomp violation or if it crashed for some other reason.
Furthermore, `oidc-agent` forks another process for some operations. These might
be killed as well without effecting the actually `oidc-agent` process. If that
happens you might experience unrelated error messages.

The name of the syscall that could not be performed can be obtained the
following way:
- You must have `auditd` installed and running. (Before the bad syscall
  happens.)
- Use [ `getBadSysCall.sh`
  ](https://github.com/indigo-dc/oidc-agent/blob/master/src/privileges/getBadSysCall.sh) to get the syscall name. 
  Call it with the name of the broken component as a parameter:
Example: `getBadSysCall.sh oidc-gen`.
  
  Note: If the httpserver of oidc-agent breaks you might have to use
  `getBadSysCall.sh MHD-listen`.

If you know the name of the violating syscall you can add it to the white-list.
The list is split into multiple files located at
`/etc/oidc-agent/privileges`. Choose the one that seems to fit best.


