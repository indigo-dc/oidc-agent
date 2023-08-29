## Communication

Because the oidc-agent project consists of multiple components and also other
applications can interface with `oidc-agent`, communication between these
components is an important part of the project.

Other applications (including `oidc-gen`, `oidc-add`, and
`oidc-token`) can communicate with `oidc-agent` through a UNIX domain
socket. This socket can be located through the `$OIDC_SOCK` environment
variable.

The socket is created when `oidc-agent` starts. The access control on that
socket is handled by the file system. The socket is created with user
privileges, allowing every application running as the same user as the user that
started `oidc-agent` to communicate with the agent.

A man-in-the-middle attack on this socket would be possible, e.g. using
`socat`. However, it requires an attacker to already have user privileges.
Also, sensitive information is encrypted.

`oidc-gen` and `oidc-add` encrypt all their communication with the agent (their communication might contain sensitive
information like user credentials (only for `oidc-gen` when using the password flow), OIDC refresh token, client
credentials, lock password, etc.)
Communication done with `liboidc-agent` (including `oidc-token`) is also encrypted.
If an application communicates directly through the UNIX domain socket with
`oidc-agent` encryption is theoretically supported.
However, it requires usage of libsodium and the implementation details (used functions, parameters, etc.) are not
documented and have to be retrieved from the source.

Internally `oidc-agent` consists of two components that communicate through unnamed
pipes. This communication is not encrypted, because it cannot be accessed by
other processes.


