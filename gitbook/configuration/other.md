## Other Configuration
Generally oidc-agent does not use any configuration files. (Configuration files
for [seccomp](../security/seccomp.md) and the already mentioned `issuer.conf` excluded.) 
Configuration needed is done mostly through command line options to the
different components and in some cases through environement variables.
If some command line options are used for every call, it makes sense to define
an alias for it in `.bashrc` or `.bash_aliases`, e.g. `alias oidc-add="oidc-add --pw-store=3600"`.
