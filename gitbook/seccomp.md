# seccomp

With version 2.0.0 we integrated ```seccomp``` into oidc-agent to restrict the system calls that each component is allowed to perform. 
However, the system calls needed may vary for different systems and architectures. 
Therefore, it is possible that you might face errors saying 'Bad Syscall'. 
If you get such an error please hand in a bug report to help improving oidc-agent further. 
You can also turn seccomp off by using the ```--no-seccomp``` option.

Note: If oidc-agent is killed due to a bad syscall you most likely won't be able
to see the 'Bad Syscall' message.

## A Seccomp Bug Report

Seccomp bugs can be very hard to debug. Therefore, it is important to give us as
many useful information as possible.

A seccomp bug report should include the following details:
- oidc-agent version: Can be obtained by running ```oidc-agent -V```
- a discription of what you were doing
- the name of the component that broke (oidc-agent, oidc-gen, oidc-add, or
  oidc-token)
- the syscall that could not be performed

The name of the syscall that could not be performed can be obtained the
following way:
- You must have ```auditd``` installed and running. (Before the bad syscall
  happens.)
- Use [ ```getBadSysCall.sh```
  ](https://github.com/indigo-dc/oidc-agent/blob/src/privileges/getBadSysCall.sh) to get the syscall name. 
  Call it with the name of the broken component as a parameter:
Example: ```getBadSysCall.sh oidc-gen```.
