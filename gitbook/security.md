# Security

## Account Configuration Files
The generated account configuration files contain sensitive information (i.e.
client credentials and the refresh token) and are therefore stored in an
encrypted way.

All encryption done in the ```oidc-agent``` project are done through the
[```libslibsodium library```](https://github.com/jedisct1/libsodium), which is
also used by software such as ```Discord```, ```RavenDB```, or ```Wire```.

The encryption uses an ```XSalsa20``` stream cipher.

## Credentials
(User) credentials are very sensitive information and have to be handled with
adequate caution.

### User Credentials
The user only has to pass its credentials (for the OpenID Provider) to
```oidc-agent``` when using the password flow. This flow has to be explicilty enabled
to use it with ```oidc-gen```. Furthermore, it is not even supported by most
providers and if it might require manual approval from an OpenID Provider
administrator.
However, when user credentials are passed to ```oidc-agent``` we handle them
carefully and keep them as short as possible in memory. Credentials are also
overwritten before the memory is freed (see also [memory](#memory)) and never
stored on disk.

### Refresh Tokens
OpenID Connect refresh tokens can be used to obtain additional access tokens and
must be kept secret. The refresh token is stored encrypted in the account configuration file (s. [account configuration files](#account-configuration-files)). The refresh token is only read by ```oidc-gen``` (during account configuration generation) and ```oidc-add``` (when adding a configuration to the agent). When using the autoload feature (see [account autoload](#account-autoload)) also ```oidc-agent``` reads the refresh token from the encrpyted account configuration file. However, ```oidc-gen``` and ```oidc-add``` do not use the refresh token, they only pass it to ```oidc-agent```. ```oidc-agent``` uses the refresh token to obtain additional access tokens. 
**The agent has to keep the refresh token in memory. However, when not beeing
used it will be obfuscated, so it is harder to extract it from a memory dump.
The password used for this obfuscation is dynamically generated when the agent
starts.**

**The refresh token cannot be requested from ```oidc-agent```and is never send to any other application.**

## Memory
Programming in ```C``` always requires caution when it comes to memory security.
Because we handle sensitive data, we decided to clear all allocated memory
before freeing it. To do this we wrote our own memory allocator (wrapper) and a
custom free. By clearing all allocated memory and not only the parts known to be
sensitive we ensure that all sensitive data is overwritten before freed. (Even if
there is a refresh token as part of a server response.) 
Sensitive data on the stack is explicilty overwritten after usage.

Refresh tokens and client credentials are the most sensitive information that
have to be kept in memory by ```oidc-agent``` for the whole time. To make it
harder for an attacker to extract this information from the agent, it is
obfuscated when not beeing used. The password used for obfuscation is
dynamically generated when the agent starts.
Additional encryption is applied when the agent is locked (see [Agent
Locking](#agent-locking)).

## Agent Locking
The agent can be locked using a locking password. While being locked the only
operation allowed are:
- checking if the agent is running
- unlocking the agent
Every other request will result in an error ```Agent locked```. This allows a
user to temporarily forbid all operations / requests without removing the loaded
account configurations.

When the agent is locked refresh tokens, access tokens, and client credentials are encrypted using the locking password provided by the user.

The agent also offers brute force protection. When trying to unlock the agent
with a wrong password a small delay is added, which will increase with the
number of failed attempts up to 10 seconds.

## ```seccomp```
With version 2.0.0 we integrated ```seccomp``` into ```oidc-agent``` to restrict the system calls that each component is allowed to perform. 
However, the system calls needed may vary for different operating system (versions) and architectures. Therefore, we decided to disable this feature by default.
(expert) users with higher security standards can turn seccomp on by using the ```--seccomp``` option. (It does make sense to define aliases.) However, these users will have to maintain the whitelisted system calls on their own. The configuration files are located in ```/etc/oidc-agent/privileges/```
Because seccomp can not restrict string parameters (e.g. paths) the added security is rather small. This is because even though privilege separation is in place and e.g. ```oidc-agent``` does not have to read and write regular files, it still needs system calls that can be sued to read and write regular files, because ```oidc-agent``` also needs to create a tmp directory, create sockets, read and write from sockets and pipes, read time and random, ...

Users that want to restrict ```oidc-agent``` further might want to have a look
at [AppArmor](https://help.ubuntu.com/lts/serverguide/apparmor.html.en).

### Missing Seccomp System Calls

A missing system call can be very hard to debug. The first step is to ensure
that there really is a missing system call. For all client components (```oidc-add```,
```oidc-gen```, etc.) this is very easy. If they are killed by the kernel you probably
will see a 'Bad Syscall' message on your terminal. However, for ```oidc-agent``` this
message will not appear so it's not that easy to tell if ```oidc-agent``` was killed
beacuse of a seccomp violation or if it crashed for some other reason.
Furthermore, ```oidc-agent``` forks another process for some operations. These might
be killed as well without effecting the actually ```oidc-agent``` process. If that
happens you might experience unrelated error messages.

The name of the syscall that could not be performed can be obtained the
following way:
- You must have ```auditd``` installed and running. (Before the bad syscall
  happens.)
- Use [ ```getBadSysCall.sh```
  ](https://github.com/indigo-dc/oidc-agent/blob/master/src/privileges/getBadSysCall.sh) to get the syscall name. 
  Call it with the name of the broken component as a parameter:
Example: ```getBadSysCall.sh oidc-gen```.
  
  Note: If the httpserver of oidc-agent breaks you might have to use
  ```getBadSysCall.sh MHD-listen```.

If you know the name of the violating syscall you can add it to the whitelist.
The list is splitted into multiple files located at
```/etc/oidc-agent/privileges```. Choose the one that seems to fit best.

## Final Note
While we do our best to make it as hard as possible to extract any sensitive
information from ```oidc-agent```, it is impossible to fully protect any
application from an attacker that has the same or even higher rights as the user
that runs the application (i.e. ```oidc-agent```).
