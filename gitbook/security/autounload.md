## Autounload (Lifetime)

Generally, we keep all information in memory as short as possible, but sometimes we have to keep information for a
longer time, e.g. the account configuration.
Loaded account configurations can be automatically unloaded after a user-defined timespan.
A user can use the lifetime option to control how long a configuration will live in the agent, after that time it is
automatically unloaded.
This feature plays very well with the autoload feature, because it makes it easy to use small lifetimes on default,
because an unloaded configuration can be loaded again into the agent without running oidc-add, but simply when it is
required.
If this is combined with an `gpg`-based encryption the experience can be further improved.
