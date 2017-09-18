# User Guide
## Quickstart for INDIGO IAM
Once installed a account configuration for INDIGO IAM can be generated. 

The next lines will start oidc-agent and the client registration process.
```
eval `oidc-agent`
oidc-gen <shortname>
```
A client will be registered, but it misses the password grant type.
Contact an INDIGO IAM admin to update the client configuration. 
All needed information are printed by oidc-gen.

After the client configuration was updated by an admin, the account 
configuration generation can be finished. 

Provide the clientconfig file to oidc-gen to do so.
```
oidc-gen -f <path_to_clientconfigfile>
```
Now the account configuration was created and already added to the agent. 

Test it with:
```
oidc-token <shortname>
```

After this initial generation the account configuration can be added to the started agent using oidc-add:
```
oidc-add <shortname>
```


## oidc-agent
You can start the agent by running:
```
oidc-agent
```
This will print out shell commands which have to be executed in the shell where
you want to run oidc-add, oidc-gen, and any client.

To start oidc-agent and directly set the needed environment variables you can use:
```
eval `oidc-agent`
```

Using oidc-agent is made as easy as possible. In case you are lost oidc-agent provides 
a lot of information with its 'help' command, just call `oidc-agent --help`.
```
$ oidc-agent --help
Usage: oidc-agent [OPTION...]
oidc-agent -- A agent to manage oidc token

  -c, --console              runs oidc-agent on the console, without daemonizing
  -g, --debug                sets the log level to DEBUG
  -k, --kill                 Kill the current agent (given by the OIDCD_PID environment variable).
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

## oidc-gen
You can use oidc-gen to generate a new agent account configuration. 

```
$ oidc-gen --help
Usage: oidc-gen [OPTION...] [SHORT_NAME]
oidc-gen -- A tool for generating oidc account configurations which can be used
by oidc-add

  -d, --delete               delete configuration for the given account
  -f, --file=FILE            specifies file with client config. Implicitly sets -m
  -g, --debug                sets the log level to DEBUG
  -m, --manual               Does not use Dynamic Client Registration
  -o, --output=OUTPUT_FILE   the path where the client config will be saved
  -v, --verbose              enables verbose mode. 
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

Most likely you do not have already a client registered and don't want to do it through a web 
interface. If the provider supports dynamic registration, you can let the agent
register a new client for you. This is the default option and you can run ```oidc-gen``` to do so. 

Using iam password grant type is not supported using dynamic registration. The client is registered
and you have to contact the provider to update the client config manually. After that is
done, you can specify the saved client config file to oidc-gen using ```oidc-gen -f <filepath>```
and finish the account configuration. Afterwards the config is added to oidc-agent 
and can be used by oidc-add normally to add and remove the account configuration from the agent.

If you have already a registered client (e.g. because the provider does not support dynamic registration) you can run 
```oidc-gen -m``` for manual configuration. oidc-gen will prompt you for the relevant
information. If you have a file with client configuration information you can pass it to oidc-gen using the ```-f``` flag. 

If you want to edit an existing configuration, you can do so by running oidc-gen
and providing the short name for this configuration.

oidc-gen will also add the generated configuration to the agent. So you don't
have to run oidc-add afterwards. However, if you want to load an existing
configuration oidc-add is your friend.


## oidc-add
oidc-add will add an existing configuration to the oidc-agent. 
```
$ oidc-add --help
Usage: oidc-add [OPTION...] ACCOUNT_SHORTNAME
oidc-add -- A client for adding and removing accounts to the oidc-agent

  -g, --debug                sets the log level to DEBUG
  -r, --remove               the account config is removed, not added
  -v, --verbose              enables verbose mode. The send data will be
                             printed.
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

One has to provide the short name of the account configuration via command line
argument.
```
oidc-add <shortname>
```

## oidc-token
oidc-token is an example agent client using the provided C-API and can be used to 
easily get an OIDC access token from the command line. oidc-token can also list the
currently loaded accounts.

```
$ oidc-token --help
Usage: oidc-token [OPTION...] ACCOUNT_SHORTNAME | -l
oidc-token -- A client for oidc-agent for getting OIDC access tokens.

  -l, --listaccounts         Lists the currently loaded accounts
  -t, --time=min_valid_period   period of how long the access token should be at least valid in seconds
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

To get an access token for an account you have to specify the short name and
how long the access token should be valid at least. The time is given in
seconds. If no minimum period of validity is specified, the default value 0 will
be used. This means that the access token might not be valid anymore even when
be used instantly. If the current access token is not valid long enough, a new 
access token is issued and returned. We guarantee that the token will be valid 
the specific time, if it is below the provider's maximum, otherwise it will be the 
provider's maximum.

The following call will get an access token for the account with the short name
'iam'. The access token will be valid at least for 60 seconds.
```
oidc-token iam -t 60
```

## Other agent clients
Any application that needs an access token can use our API to get an access token from 
oidc-agent. The following applications are already able to get an access token from oidc-agent:
- [wattson](https://github.com/indigo-dc/wattson)
- [orchent](https://github.com/indigo-dc/orchent)
