# Security

## Privilege Separation & Architecture
We followed the security by design principle and split the system’s functionalities
into multiple components. In that way we also achieved privilege separation.

The oidc-agent project consists of the following components:
- ```oidc-agent```: The actual agent managing the tokens and performing all communication
with the OpenID Provider; internally also has two components:
  - oidc-agent-proxy: A proxy daemon that forwards requests to
    oidc-agent-daemon. It handles encryption passwords and file access for oidc-agent-daemon when it has to read (autoload) or write (changing refresh token) an account configuration file.
  - oidc-agent-daemon: The daemon that holds the loaded accounts and performing
    all communication with the OpenID Provider
- ```oidc-gen```: A tool for generating account configuration files for usage with ```oidc-agent``` and
```oidc-add```.
- ```oidc-add```: A tool that loads the account configurations into the agent.
- ```oidc-token``` and third party applications: Applications that need an OIDC access token
can obtain it through the agent’s [API](api.md). One
example application for obtain access tokens is ```oidc-token```.

![Architecture of the oidc-agent project](https://raw.githubusercontent/indigo-dc/oidc-agent/master/gitbook/images/architecture.png)

### Privileges Needed by Different Components
The following list might not be complete when it comes to implementation details
(e.g. privileges needed for obtain the current time).

Privileges for the differen components:
- oidc-agent-daemon:
  - pipe ipc
  - socket ipc
  - internet
    - also reads the CA bundle file
  - starts web server
  - reads time
  - reads random
- oidc-agent-proxy:
  - creates directory in ```/tmp```
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
  - writes files in ```/tmp```
  - writes files passed to ```--output```
  - reads files passed to ```--file```, ```--print```
  - reads random
  - socket ipc
  - might call ```xdg-open``` to open the authorization url in a browser
  - might execute the user provided ```--pw-cmd```
- oidc-add:
  - reads files in the oidc-agent directory
  - reads random
  - socket ipc
  - might execute the user provided ```--pw-cmd```
- oidc-token:
  - socket ipc

## Account Configuration Files
The generated account configuration files contain sensitive information (i.e.
client credentials and the refresh token) and are therefore stored in an
encrypted way.

All encryption done in the ```oidc-agent``` project are done through the
[```libsodium library```](https://github.com/jedisct1/libsodium), which is
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

## Communication
Because the oidc-agent project consists of multiple components and also other
applications can interface with ```oidc-agent```, communication between these
components is an important part of the project.

Other applications (including ```oidc-gen```, ```oidc-add```, and
```oidc-token```) can communicate with ```oidc-agent``` through a UNIX domain
socket. This socket can be located through the ```$OIDC_SOCK``` environment
variable.

The socket is created when ```oidc-agent``` starts. The access control on that
socket is handled by the file system. The socket is created with user
privileges, allowing every application running as the same user as the user that
started ```oidc-agent``` to communicate with the agent.

A man-in-the-middle attack on this scoket would be possible, e.g. using
```socat```. However, it requires an attacker to already have user privileges.
Also sensitive information is encrypted.

```oidc-gen``` and ```oidc-add``` encrypt all their communication with the agent (their communication might contain sensitive informataion like user credentials (only for ```oidc-gen``` when using the password flow), OIDC refresh token, client credentials, lock password, etc.)
Communication done with ```liboidc-agent``` (including ```oidc-token```) is not encrypted. However, this communication only contains OIDC access token as sensitive information and these can be requested by any application with access to the agent socket.
If an application communciates directly thorugh the UNIX domain socket with
```oidc-agent``` encryption is theoretically supported.
However, it requires usage of libsodium and the implementation details (used functions, parameters, etc.) are not documented and have to be retrieved from the source.

Internally ```oidc-agent``` has two components that communicate through unnamed
pipes. This communication is not encrypted, because it cannot be accessed by
other processes.

## Encryption Passwords
Generally, the encryption password provided by the user to encrypt / decrypt
account configurations is kept in memory as short as possible. However, ```oidc-agent``` can keep them in memory (encrypted) so it can update the configuration file when a provider issues a new refresh token. This option is not enabled on default and has to enabled explicitly by the user.

If the user did not enable any password caching feature and ```oidc-agent``` needs an encryption password because it has to update the account configuration file, the user will be prompted for the password. However, if a provider uses rotating refresh tokens, this might be inpractical, because the user has to enter his encryption password whenever a new access token is issued. We therefore implemented different password caching features:
- ```oidc-agent``` can keep the encryption password in memory. The encryption password will be well encrypted; and the password used for this encryption is dynamically generated when the agent starts. The time how long the encryption password should be kept in memory can also be limited, i.e. after that time the password will be cleared from memory.
- ```oidc-agent``` can also save the encryption password in the system's password manager (keyring). Because any other application running as the same user then has access to this password, ```oidc-agent``` still applies encryption on the user's password, even though the keyring will save the password encrypted. However, to prevent other applications to obtain the plain password, we only store the encrypted password.
- ```oidc-agent``` can also retrieve the encryption password from a user provided command; the ouput of this command will be used as the encryption password. The command used will be kept encrypted in memory, because it is used to obtain the encryption password without any additional checks, it should be treated the same way as the encryption password itself. Because with this option the user does not have to enter his password at any point (also not when loading the account configuration with ```oidc-add```) it might be especially usefull when writing scripts.

## Autounload (Lifetime)
Generally, we keep all information in memory as short as possible, but sometimes we have to keep information for a longer time, e.g. the account configuration.
As well as an encryption password that is kept encrypted in memory can automatically removed after a specified time, an account configuration can also be removed after some time.
A user can use the lifetime option to control how long a configuration will live in the agent, after that time it is automatically unloaded.
This feature plays very well with the autoload feature, because it makes it easy to use small lifetimes on default, because an unloaded configuration can be loaded again into the agent without running oidc-add, but simply when it is required, but it still requires user interaction.

## User Confirmation
When loading an account configuration into the agent using ```oidc-add``` a user can specify that he wants to confirm each usage of this configuration. Therefore, an application can only obtain an access token from the agent if the user approves it.

## Tracing
We disabled the possibility to attach to any oidc-agent component with tracing.
(Among others, tracing can be used to get a memory dump). However, this only holds for non privileged users, as root it is still possible to trace oidc-agent and to obtain memory dumps.

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
