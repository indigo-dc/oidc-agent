# oidc-agent
## Starting oidc-agent
You can start the agent by running:
```
oidc-agent
```
This will print out shell commands which have to be executed in the shell where
you want to run oidc-add, oidc-gen, and any application using oidc-agent.

To start oidc-agent and directly set the needed environment variables you can use:
```
eval `oidc-agent`
```

## Persistence of oidc-agent
## General Usage
```
$ oidc-agent --help
Usage: oidc-agent [OPTION...] 
oidc-agent -- An agent to manage oidc token

 General:
  -c, --confirm              Require user confirmation when an application
                             requests an access token for any loaded
                             configuration
  -k, --kill                 Kill the current agent (given by the OIDCD_PID
                             environment variable)
      --no-autoload          Disables the autoload feature: A token request
                             cannot load the needed configuration. The user has
                             to do it with oidc-add.
      --seccomp              Enables seccomp system call filtering; allowing
                             only predefined system calls.
  -t, --lifetime=TIME        Set a default value in seconds for the maximum
                             lifetime of account configurations added to the
                             agent. A lifetime specified for an account
                             configuration with oidc-add overwrites this
                             default value. Without this option the default
                             maximum lifetime is forever.

 Verbosity:
  -d, --console              Runs oidc-agent on the console, without
                             daemonizing
  -g, --debug                Sets the log level to DEBUG

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <https://github.com/indigo-dc/oidc-agent/issues>
Subscribe to our mailing list to receive important updates about oidc-agent:
<https://www.lists.kit.edu/sympa/subscribe/oidc-agent-user>.
```

The ```-t``` option can be used to set a default lifetime for all loaded account
configurations. This way all account configurations will only be loaded for a
limit time after which they are automatically removed from the agent. 
This option can be overwritten by the valud passed to ```oidc-add -t``` when
loading the configuration. 

