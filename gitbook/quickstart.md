# Quickstart

After [installation](installation/install.md) the agent has to be started. Usually the agent is started on system
startup and is then available on all terminals (see [integration](configuration/integration.md)). Therefore, after
installation the options are to restart your X-Sessio or to start the agent manually.

```
eval `oidc-keychain`
```

This starts the agent and sets the required environment variables.

## Create an agent account configuration with oidc-gen

For most OpenID Connect providers an agent account configuration can be created with one of the following calls. Make
sure that you can run a web-browser on the same host where you run the `oidc-gen` command.

```
oidc-gen <shortname>
oidc-gen --pub <shortname>
```

For more information on the different providers refer to [integrate with different providers](provider/provider.md).

After an account configuration is created it can be used with the shortname to obtain access tokens. One does not need
to run `oidc-gen` again unless to update or create a new account configuration.

## Use oidc-add to load an account configuration

```
oidc-add <shortname>
```

However, usually it is not necessary to load an account configuration with
`oidc-add`. One can directly request an access token for a configuration and
`oidc-agent` will automatically load it if it is not already loaded.

## Obtaining an access token

```
oidc-token <shortname>
```

Alternatively, it is also possible to request an access token without specifying the shortname of a configuration but
with the issuer url:

```
oidc-token <issuer_url>
```

This way is recommended when writing scripts that utilize oidc-agent to obtain access tokens. This allows that the
script can be easily used by others without them having to update the shortname.

## List existing configuration

```
oidc-add -l
oidc-gen -l
```

These commands both give a list of all existing account configurations.

A list of the currently loaded accounts can be retrieved with:

```
oidc-add -a
```

## Updating an existing account configuration

An existing account configuration can be updated with `oidc-gen`:

```
oidc-gen -m <shortname>
```

## Reauthenticating

If the refresh token stored in the account configuration expired a new one must be created. However, it is not required
to create a new account configuration, it is enough to run:

```
oidc-gen <shortname> --reauthenticate
```