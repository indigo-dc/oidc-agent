# oidc-agent
oidc-agent is a program to manage OpenID Connect tokens. It design is strongly
oriented at ssh-agent, so users can handle oidc tokens in a similiar way as they
do with ssh keys. oidc-agent is usually started in the beginning of
an X-session or a login session, and all other windows or programs are started as 
clients to the ssh-agent program. Through use of environment variables the agent 
can be located and automatically used to handle oidc tokens.

The agent initially does not have any providers loaded.  You can load a
provider configuration by using oidc-add.  Multiple provider configurations may
be loaded in oidc-agent concurrently.  oidc-add is also used to remove a loaded
configuration from oidc-agent.

## Requirements:
  - libcurl: https://curl.haxx.se/libcurl/ debian-package: libcurl4-openssl-dev
  - libsodium: https://download.libsodium.org/doc/ debian-package:
    libsodium-dev

If you have the required libraries installed you should be able to compile by
running ```make```

## Configuration
The oidc directory will be created by make. If ~/.config exists it will be ~/.config/oidc otherwise ~/.oidc
there will be a config file in the near future.

## Usage

### oidc-agent
You can start the agent by running:
```
./bin/oidc-agent
```
This will print out shell commands which have to be executed in the shell where
you want to run oidc-add, oidc-gen, and any client.

To start oidc-agent and directly set the needed environment variables you can use:
```
eval `./bin/oidc-agent`
```

### oidc-gen
You can use oidc-gen to generate a new oidc provider config. When running
```./bin/oidc-gen``` the program will prompt you for the relevant
information. 
If you want to edit an existing configuration, you can do so by running oidc-gen
and providing the short name for this configuration.

oidc-gen will also add the generated configuration to the agent. So you don't
have to run oidc-add afterwards. However, if you want to load an existing
configuration oidc-add is your friend.

### oidc-add
oidc-add will add an existing configuration to the oidc-agent, making it useable. You
have to provide the short name of the provider configuration via command line
argument.
```
./oidc-add/bin/oidc-add <shortname>
```

### clients
clients can use the provided api to communicate with oidc-agent. An example client is
oidc-token. It can list the currently loaded providers and get an access token.

For displaying a list of loaded providers run
```
./api/bin/oidc-token -l
```

To get an access token for one provider you have to specify the short name and
how long the access token should be valid at least. The time is given in
seconds. If no minimum period of validity is specified, the default value 0 will
be used. This means that the access token might not be valid anymore even when
be used instantly. 
The following call will get an access token for the provider with the short name
'iam'. The access token will be valid at least for 60 seconds.
```
./api/bin/oidc-token iam -t 60
```

