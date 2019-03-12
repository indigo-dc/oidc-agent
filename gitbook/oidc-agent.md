# oidc-agent

oidc-agent is the central comment of the oidc-agent tools. It manages all OpenID
Connect tokens are communicates with the OpenID Providers.
Other applications can request access tokens from the agent.

## Starting ```oidc-agent```
As described in [Xsession integration](configure.md#xsession-integration) on
default oidc-agent is integrated with Xsession. Therefore, it is automatically
started and available in all terminals through that session. So usually a user
does not have to start oidc-agent. 

After installing oidc-agent the agent will not be automatically available. After
a system restart the agent can be used in all terminals.

The agent can also be started by using:
```
oidc-agent
```
This will print out shell commands which have to be executed in the shell where
you want to run oidc-add, oidc-gen, and any application using oidc-agent.

To start oidc-agent and directly set the needed environment variables you can use:
```
eval `oidc-agent`
```

## General Usage
```
$ oidc-agent --help
Usage: oidc-agent [OPTION...] 
oidc-agent -- An agent to manage oidc token

 General:
  -c, --confirm              Requires user confirmation when an application
                             requests an access token for any loaded
                             configuration
  -k, --kill                 Kill the current agent (given by the OIDCD_PID
                             environment variable)
      --no-autoload          Disables the autoload feature: A token request
                             cannot load the needed configuration. The user has
                             to do it with oidc-add.
      --no-webserver         This option applies only when the authorization
                             code flow is used. oidc-agent will not start a
                             webserver. Redirection to oidc-gen through a
                             custom uri scheme redirect uri and 'manual'
                             redirect is possible.
      --pw-store[=TIME]      Keeps the encryption passwords for all loaded
                             account configurations encrypted in memory for
                             TIME seconds. Can be overwritten for a specific
                             configuration with oidc-add.Default value for
                             TIME: Forever
      --seccomp              Enables seccomp system call filtering; allowing
                             only predefined system calls.
  -t, --lifetime=TIME        Sets a default value in seconds for the maximum
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

## Detailed Information About All Options

* [```--confirm```](#--confirm)
* [```--console```](#--console)
* [```--debug```](#--debug)
* [```--kill```](#--kill)
* [```--no-autoload```](#--no-autoload)
* [```--no-webserver```](#--no-webserver)
* [```--pw-store```](#--pw-store)
* [```--seccomp```](#--seccomp)
* [```--lifetime```](#--lifetime)

### ```--confirm```
On default every application running as the same user as the agent can obtain an
access token for every account configuration from the agent. The ```--confirm```
option can be used to change this behavior. If that option is used, the user has
to confirm each usage of an account configuration, allowing fine grained control
from the user. The ```--confirm``` option can be used when loading an account
configuration through ```oidc-add```, in that case only that specific account needs
confirmation, or when starting the agent. If the option is used with the agent,
every usage of every account configuration has to be approved by the user.

### ```--console```
Usually oidc-agent runs in the background as a daemon. This option will skip
the daemonizing and run on the console. This might be sued for debugging.

### ```--debug```
This increases the log level to ```DEBUG``` and obviously should only be used to
debug porpuses. If enabled, sensitive information (among others refresh tokens and client
credentials) are logged to the system log.

### ```--kill```
This will kill the currently running agent. The agent to be killed is identified
by the ```OIDCD_PID``` environment variable. When integrated with Xsession this
will kill the agent available in all terminals. A restarted agent will not
automatically be available in already existing or new terminals. You can use
your [```.bashrc```](configure.md#persisting-oidc-agent-through-.bashrc) to make a newly started agent available in new terminals.

### ```--no-autoload```
On default account configurations can automatically be loaded if needed. That means
that an application can request an access token for every account configuration.
If it is not already loaded the user will be prompted to enter the needed
encryption password. After the user enters the password the account configuration
is loaded and an access token returned to the application. the user can also
cancel the autoload.

With ```--no-autoload``` enabled the agent will not load currently not loaded account configuration for which an access token is requested. The user then first has to add them manually by using ```oidc-add```, before an application can obtain an access token for those.

### ```--no-webserver```
This option can be used when the authorization code flow is performed. On default a small
webserver is started by ```oidc-agent``` to be able to catch the redirect and
complete the authorization code flow. The ```--no-webserver``` option tells
```oidc-agent``` that no webserver should be started (for any account
configuration). The authorization code
flow can still be completed. Either by using a redirect uri that follows the
custom redirect uri scheme ```edu.kit.data.oidc-agent:/<path>``` - this will
directly redirect to oidc-gen, or by copying the url the browser would normally
redirect to and pass it to ```oidc-gen --codeExchange```.

### ```--pw-store```
When this option is provided, the encryption password for all account
configurations  will be kept in memory by
oidc-agent (in an encrypted way).

This option can also be sued with ```oidc-add```. When this option is used with
```oidc-agent``` it applies to all loaded account configuration; when used with
```oidc-add``` only for that specific one. See [```oidc-add
--pw-store```](oidc-add.md#--pw-store) for more information.

### ```--seccomp```
Enables seccomp system call filtering. See [general seccomp
notes](security.md#seccomp) for more details.

### ```--lifetime```
The ```--lifetime``` option can be used to set a default lifetime for all loaded account
configurations. This way all account configurations will only be loaded for a
limited time after which they are automatically removed from the agent. 
When loading an account configuration with ```oidc-add``` this lifetime can be
overwritten. So that a specific account configuration can be loaded with another
lifetime (lower, higher, and also infinite).

Using ```--lifetime=0``` means that account configuration are not automatically
removed and they are kept loaded for an infinte time. This is also the default
behavior.
