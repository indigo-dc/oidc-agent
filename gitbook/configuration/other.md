## Other Configuration

Generally oidc-agent does not use any configuration files.
(For the cases where configuration files are used they are placed in `/etc/oidc-agent/` or in
the [agent directory](../directory.md))
Configuration needed is done mostly through command line options to the different components and in some cases through
environment variables. If some command line options are used for every call, it makes sense to define an alias for it
in `.bashrc` or `.bash_aliases`, e.g. `alias oidc-add="oidc-add --pw-store=3600"`.
