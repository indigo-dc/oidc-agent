## Memory
Programming in `C` always requires caution when it comes to memory security.
Because we handle sensitive data, we decided to clear all allocated memory
before freeing it. To do this we wrote our own memory allocator (wrapper) and a
custom free. By clearing all allocated memory and not only the parts known to be
sensitive we ensure that all sensitive data is overwritten before freed. (Even if
there is a refresh token as part of a server response.) 
Sensitive data on the stack is explicitly overwritten after usage.

Refresh tokens and client credentials are the most sensitive information that
have to be kept in memory by `oidc-agent` for the whole time. To make it
harder for an attacker to extract this information from the agent, it is
obfuscated when not being used. The password used for obfuscation is
dynamically generated when the agent starts.
Additional encryption is applied when the agent is locked (see [Agent
Locking](agent-locking.md)).


