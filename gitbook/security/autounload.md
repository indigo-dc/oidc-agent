## Autounload (Lifetime)
Generally, we keep all information in memory as short as possible, but sometimes we have to keep information for a longer time, e.g. the account configuration.
As well as an encryption password that is kept encrypted in memory can automatically removed after a specified time, an account configuration can also be removed after some time.
A user can use the lifetime option to control how long a configuration will live in the agent, after that time it is automatically unloaded.
This feature plays very well with the autoload feature, because it makes it easy to use small lifetimes on default, because an unloaded configuration can be loaded again into the agent without running oidc-add, but simply when it is required, but it still requires user interaction.


