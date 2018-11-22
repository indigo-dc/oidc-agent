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
A simple way to make oidc-agent persistent is to include this line in your
`.bashrc`:
```
test -e ~/tmp/oidc-agent.env && . ~/tmp/oidc-agent.env
```
And to run the agent as `oidc-agent > ~/tmp/oidc-agent.env`
From now on every new shell should have access to the agent. 

You can test this with:
```
oidc-token <shortname>
```

## General Usage
```
$ oidc-agent --help
Usage: oidc-agent [OPTION...] 
oidc-agent -- An agent to manage oidc token

 General:
  -k, --kill                 Kill the current agent (given by the OIDCD_PID
                             environment variable)
      --seccomp              Enables seccomp system call filtering; allowing
                             only predefined system calls.
  -t, --lifetime=LIFETIME    Set a default value in seconds for the maximum
                             lifetime of account configurations added to the
                             agent. A lifetime specified for an account
                             configuration with oidc-add overwrites this
                             default value. Without this option the default
                             maximum lifetime is forever.

 Verbosity:
  -c, --console              Runs oidc-agent on the console, without
                             daemonizing
  -g, --debug                Sets the log level to DEBUG

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <https://github.com/indigo-dc/oidc-agent/issues>.
```

The ```-t``` option can be used to set a default lifetime for all loaded account
configurations. This way all account configurations will only be loaded for a
limit time after which they are automatically removed from the agent. 
This option can be overwritten by the valud passed to ```oidc-add -t``` when
loading the configuration. 

