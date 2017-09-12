# oidc-agent
oidc-agent is a program to manage OpenID Connect tokens and make them easily usable 
from the command line. It design is strongly oriented at ssh-agent, so users can 
handle oidc tokens in a similiar way as they do with ssh keys. 

![one does simply](one_does_simply.jpg)

oidc-agent is usually started in the beginning of
an X-session or a login session, and all other windows or programs are started as 
clients to the oidc-agent program. Through use of environment variables the agent 
can be located and automatically used to handle oidc tokens.

The agent initially does not have any configurations loaded.  You can load a
account configuration by using oidc-add.  Multiple accounts configurations may
be loaded in oidc-agent concurrently.  oidc-add is also used to remove a loaded
configuration from oidc-agent.

## Requirements:
  - libcurl: https://curl.haxx.se/libcurl/ debian-package: libcurl4-openssl-dev
  - libsodium: https://download.libsodium.org/doc/ debian-package:
    libsodium-dev

If you have the required libraries installed you should be able to compile and 
install by running 
```
make && sudo make install
``` 

## Configuration
The oidc directory will be created by make. If ```~/.config``` exists it will be ```~/.config/oidc-agent``` otherwise ```~/.oidc-agent```

You can configure issuer in the config file issuer.config located in this oidc directory. Simply put often used issuers in this config file, one issuer per line. They will be used by oidc-gen as a suggestion. 

## Usage

### oidc-agent
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

### oidc-gen
You can use oidc-gen to generate a new oidc account config. 
Most likely you do not have already a client registered and don't want to do it through a web 
interface. If the provider supports dynamic registration (iam does), you can let the agent
register a new client for you. This is the default option. You can run ```oidc-gen``` to start this flow. Using iam 
password grant type is not supported using dynamic registration. The client is registered
and you have to contact the provider to update the client config manually. After that is
done, you can specify the saved client config file to oidc-gen using ```oidc-gen -f <filepath>```
and finish the account configuration. Afterwards the config is added to oidc-agent 
and can be used by oidc-add normally to add and remove the account configuration from the agent.

If you have already a registered client (e.g. because the provider does not support dynamic registration) you can run 
```oidc-gen -m``` for manual configuration. oidc-gen will prompt you for the relevant
information. 

If you want to edit an existing configuration, you can do so by running oidc-gen
and providing the short name for this configuration.

oidc-gen will also add the generated configuration to the agent. So you don't
have to run oidc-add afterwards. However, if you want to load an existing
configuration oidc-add is your friend.


### oidc-add
oidc-add will add an existing configuration to the oidc-agent, making it useable. You
have to provide the short name of the account configuration via command line
argument.
```
oidc-add <shortname>
```

### api & clients
clients can use the provided api to communicate with oidc-agent. An example client is
oidc-token. 

The api provides functions for getting a list of currently loaded account configs and access token. They can be easily used. Alternative a client can directly communicate with the oidc-agent through UNIX domain sockets. The socket address can be get from the environment variable which is set by the agent. The request has to be sent json encoded. We use a UNIX domain socket of type ```SOCK_SEQPACKET```.
The following fields and values have to be present for the different calls:

#### List of Accounts:

##### Request
| field   | value         |
|---------|---------------|
| request | account_list |

example:
```
{"request":"account_list"}
```

##### Response
| field         | value                 |
|---------------|-----------------------|
| status        | success               |
| account_list | JSON Array of strings |

example:
```
{"status":"success", "account_list":["iam", "test"]}
```

##### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | <error_description> |

example:
```
{"status":"failure", "error":"Bad Request: could not parse json"}
```

#### Access Token:
##### Request
| field            | value                  |
|------------------|------------------------|
| request          | access_token           |
| account         | <account_shortname>   |
| min_valid_period | <min_valid_period> [s] |

example:
```
{"request":"access_token", "account":"iam", "min_valid_period":60}
```

##### Response
| field        | value          |
|--------------|----------------|
| status       | success        |
| access_token | <access_token> |

example:
```
{"status":"success", "access_token":"token1234"}
```

##### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | <error_description> |

example:
```
{"status":"failure", "error":"Account not loaded"}
```

#### oidc-token
oidc-token is n example client using the provided C-api and can be used to easily get an oidc access token from the command line.

oidc-token can list the currently loaded accounts and get an access token.

For displaying a list of loaded accounts run
```
oidc-token -l
```

To get an access token for one account config you have to specify the short name and
how long the access token should be valid at least. The time is given in
seconds. If no minimum period of validity is specified, the default value 0 will
be used. This means that the access token might not be valid anymore even when
be used instantly. If the current access token is not valid long enough, a new 
access token is issued and returned. We guarantee that the token will be valid 
the specific time, if it is below the server's maximum, otherwise it will be the 
provider's maximum.

The following call will get an access token for the account with the short name
'iam'. The access token will be valid at least for 60 seconds.
```
oidc-token iam -t 60
```

