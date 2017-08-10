Requirements:
  - libcurl: https://curl.haxx.se/libcurl/
  - jsmn: https://github.com/zserge/jsmn
  - libsodium: https://download.libsodium.org/doc/

If you have the required libraries installed you should be able to compile by
running ```make```

You also have to create a oidc directy. You can create it in either of the
following locations:
- ~/.config/oidc
- ~/.oidc

## oidcd
You can start the daemon by running:
```
./bin/oidcd
```
This will print out shell commands which have to be executed in the shell were
you want to run oidc-add, oidc-gen, and any client.

To start oidcd and directly set the needed environment variables you can use:
```
eval `./bin/oidcd`
```

## oidc-gen
You can use oidc-gen to generate a new oidc provider config. When running
```./oidc-gen/bin/oidc-gen``` the program will prompt you for the relevant
information. 
If you want to edit an existing configuration, you can do so by running oidc-gen
and providing the short name for this configuration.

oidc-gen will also add the generated configuration to the daemon. So you don't
have to run oidc-add afterwards. However, if you want to load an existing
configuration oidc-add is your friend.

## oidc-add
oidc-add will add an existing configuration to the oidcd, making it useable. You
have to provide the short name of the provider configuration via command line
argument.
```
./oidc-add/bin/oidc-add -p <shortname>
```

## clients
clients can use the provided api to communicate with oidcd. An example client is
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
./api/bin/oidc-token -p iam -t 60
```

