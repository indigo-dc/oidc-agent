# oidc-add
oidc-add will add an existing configuration to the oidc-agent. It also can be
used to remove an already loaded configuration from the agent or to give a list
of all available account configurations (does not mean they are currently loaded).
```
$ oidc-add --help
Usage: oidc-add [OPTION...] ACCOUNT_SHORTNAME | -l | -x | -X | -R
oidc-add -- A client for adding and removing accounts to the oidc-agent

 General:
  -l, --list                 Lists the available account configurations
      --no-seccomp           Disables seccomp system call filtering; allowing
                             all system calls. Use this option if you get an
                             'Bad system call' error and hand in a bug report.
  -p, --print                Prints the encrypted account configuration and
                             exits
  -r, --remove               The account configuration is removed, not added
  -R, --remove-all           Removes all account configurations currently
                             loaded
  -t, --lifetime=LIFETIME    Set a maximum lifetime in seconds when adding the
                             account configuration
  -x, --lock                 Lock agent
  -X, --unlock               Unlock agent

 Verbosity:
  -g, --debug                Sets the log level to DEBUG
  -v, --verbose              Enables verbose mode

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <https://github.com/indigo-dc/oidc-agent/issues>.
```

## Loading and Unloading
To Load an account configuration to the agent one has to provide the short name of the account configuration:
```
oidc-add <shortname>
```

The ```-t``` option can be used if the configuration should automatically be
removed from the agent after a certain time. This way an account configuration
can be loaded for a limit time (e.g. 60s) after which it is automatically
removed from the agent. This option overwrites the default lifetime that might
be set for all account configuration (```oidc-agent -t```). To load an account
configuration for an infinite amount of time use ```-t 0``` (only needed if a
lifetime was set by ```oidc-agent -t``` or ```oidc-add -t```).

The ```-r``` option is used to unload an account configuration. With the
```-R``` option all loaded account configuration can be removed from the agent
with just one call.

## Locking oidc-agent
The agent can be locked using the ```-x``` option. While being locked the agent
refuses all requests. This means that no account configuration can be loaded /
unloaded and no token can be obtained from the agent. To unlock the agent again
call ```oidc-add -X```.


