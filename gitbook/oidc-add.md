# oidc-add
oidc-add will add an existing configuration to the oidc-agent. It also can be
used to remove an already loaded configuration from the agent or to give a list
of all available account configurations (does not mean they are currently loaded).
```
$ oidc-add --help
Usage: oidc-add [OPTION...] ACCOUNT_SHORTNAME | -l
oidc-add -- A client for adding and removing accounts to the oidc-agent

 General:
  -l, --list                 Lists the available account configurations and exits
  -p, --print                Prints the encrypted account configuration and exits
  -r, --remove               The account configuration is removed, not added

 Verbosity:
  -g, --debug                Sets the log level to DEBUG
  -v, --verbose              Enables verbose mode

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

One has to provide the short name of the account configuration via command line
argument.
```
oidc-add <shortname>
```


