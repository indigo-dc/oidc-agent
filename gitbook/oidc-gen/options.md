## Detailed Information About All Options

General Options:
* [`--accounts`](#accounts)
* [`--codeExchange`](#codeExchange)
* [`--confirm-default`](#confirm-default)
* [`--confirm-no`](#confirm-no)
* [`--confirm-yes`](#confirm-yes)
* [`--cp`](#cp)
* [`--delete`](#delete)
* [`--file`](#file)
* [`--flow`](#flow)
* [`--manual`](#manual)
* [`--no-scheme`](#no-scheme)
* [`--no-url-call`](#no-url-call)
* [`--no-webserver`](#no-webserver)
* [`--only-at`](#only-at)
* [`--print`](#print)
* [`--prompt`](#prompt)
* [`--pub`](#pub)
* [`--pw-cmd`](#pw-cmd)
* [`--pw-env`](#pw-env)
* [`--pw-file`](#pw-file)
* [`--pw-prompt`](#pw-prompt)
* [`--reauthenticate`](#reauthenticate)
* [`--rename`](#rename)
* [`--seccomp`](#seccomp)
* [`--update`](#update)

Options for specifying information on the command line:
* [`--at`](#at)
* [`--aud`](#aud)
* [`--client-id`](#client-id)
* [`--client-secret`](#client-secret)
* [`--cnid`](#cnid)
* [`--dae`](#dae)
* [`--issuer`](#issuer)
* [`--op-password`](#op-password)
* [`--op-username`](#op-username)
* [`--port`](#port)
* [`--redirect-uri`](#redirect-uri)
* [`--rt`](#rt)
* [`--rt-env`](#rt-env)
* [`--scope`](#scope)
* [`--scope-all`](#scope-all-and-scope-max)
* [`--scope-max`](#scope-all-and-scope-max)

### `--accounts`
Using this option `oidc-gen` will print out a list of all
configured account configurations. Configured means that they are saved on the
system and can be loaded with `oidc-add`; it does not mean that they are
currently loaded.
This option is the same as `oidc-add --list`. To show a list of the
accounts that are currently loaded use `oidc-add --loaded`.

### `--codeExchange`
When using the authorization code flow the user has to authenticate against the
OpenID Provider in a web browser and is then redirected back to the application.
To be able to catch that redirect `oidc-agent` usually starts a small webserver. If something goes wrong during the redirect (because the web server crashed or no web server was used (`--no-webserver`)) the user can still finish the account configuration generation process. In such a case the suer must copy the url he is redirected to from its browser and pass it to `oidc-gen --codeExchange`. Then oidc-gen should be able to obtain and save the final account configuration.

Note that while this option also works for `edu.kit.data.oid-cagent:/`
redirect uris, it might not be possible to obtain the uri the user is redirected
to from the browser.

### `--confim-default`
When specifying this option all confirmation/consent prompts are automatically
answered with the default value. This option is useful when you want to use `oidc-gen`
non-interactive.

Examples for confirmation prompts are:
- When creating a new account configuration using dynamic client registration
    and the process is not finished but the client already registered and the
    process is started again it is possible to reuse the already registered
    client. In that case `oidc-gen` asks to use the temporary stored data.
    (Default: yes)
- When deleting an account configuration, `oidc-gen` asks if you are sure.
    (Default: no)
- When deleting an account configuration that used dynamic client registration,
    `oidc-gen` asks to delete the client at the provider. (Default: yes)
- When deleting an account configuration, the associated refresh token will be
    automatically deleted. In case this is not possible, `oidc-gen` asks if it
    should continue or not. (Default: no)

### `--confirm-no`
When specifying this option all confirmation/consent prompts are
automatically declined. This option is useful when you want to use `oidc-gen`
non-interactive.

For examples for confirmation prompts, see [`--confirm-default`](#confirm-default).

### `--confirm-yes`
When specifying this option all confirmation/consent prompts are
automatically answered with `yes`. This option is useful when you want to use `oidc-gen`
non-interactive.

For examples for confirmation prompts, see [`--confirm-default`](#confirm-default).

### `--cp`
This option allows the user to change the CA bundle file that is used to verify
SSL/TLS certificates. A user must use this option when `oidc-gen` cannot
automatically find a proper CA bundle file (e.g. non default location). A user
can also use this option to provide a more restricted bundle file that only
contains the certificates needed for a specific provider.

### `--delete`
This option will delete the account configuration for the specified shortname.
It will also revoke the used refresh token and optionally delete the OIDC
client (not implemented yet).

### `--file`
Can be used if the client was registered manually. Most OpenID Provider allow
downloading the client configuration as a json file (or copy the `JSON` config).
Such a file can be used to reduce the amount of information passed to
`oidc-gen`. The argument should be the absolute path to a file containing
the `JSON` formatted client configuration. `oidc-gen` then reads the
client configuration from this file, so that the user does not have to enter all
information manually.

Because this option is only used when a client was registered manually, it
implicitly sets the `--manual` option.

### `--flow`
Depending on the OpenID Provider a user can use multiple OpenID/OAuth2 flows to obtain a refresh token. `oidc-agent` uses the Refresh Flow to obtain additional access token.
Therefore a refresh token is required which can be obtained in multiple ways.
The `--flow` flag can be used to enforce usage of a specific flow or to
prioritise a flow over another.
`oidc-agent` will try all flows in the following order until one succeeds:
1. Refresh Flow
2. Password Flow
3. Authorization Code Flow
4. Device Flow

Possible values for the `--flow` option are: 'refresh', 'password', 'code',
and 'device'.
The flag can also be used if multiple flows should be tried, but in a different
order than the default one. To do so provide the option multiple times with one
value per option in the desired order.

In the following we will describe the different flows:

#### Out Of Band
If a user obtained a refresh token out of band he can directly provide it to
`oidc-gen` using the `--rt` option. The
`--flow=refresh` option is then implicitly set.

**Note:** Refresh tokens are bound to a specific client id. The provided refresh
token must be issued for the provided client id.

Obtaining a valid refresh token for the specific client id is out of scope of
this documentation. We recommend one of the following flows.

#### Password Flow
Most OIDPs do not support this flow. One provider that supports the password flow is INDIGO IAM, for additional information on support of the password flow for a specific provider see the documentation for different [providers](../provider/provider.md).
The password flow can be performed using only the
command line. The credentials for the OpenID Provider have to be provided to `oidc-gen`.
The credentials are only used to obtain the refresh token and are not stored.
However, there are alternatives flows that do not reveal the user's credentials
to `oidc-agent`.

#### Authorization Code Flow
The authorization code flow is the most widely used and is therefore supported by any OpenID
Provider and does not reveal user credentials to `oidc-agent`. However, it
requires a browser on the system running `oidc-agent` and `oidc-gen`. If you don't
have a browser on that system or don't want to use it you can use the Device
Flow, if supported by the provider. If the authorization code flow is the only
flow supported by your provider and have to obtain a working account
configuration on a machine that does not have a browser (e.g. a server), you can
create the account configuration on another machine and copy / move the account
configuration file to the server.

To use the Authorization Code Flow at
least one redirect uri has to be provided (see [Redirect Uri](../provider/client-configuration-values.md#redirect-uri)).
The redirect uri must be of the scheme `http://localhost:<port>`. It is
recommend to use a port which is very unlikely to be used by any other
application (during the account generation process). Additionally multiple
redirect uris can be provided.

When starting the account generation process `oidc-agent` will try to open a
webserver on the specified ports. If one port fails the next one is tried. After
a successful startup `oidc-gen` will receive an authorization URI. When calling
this URI the user has to authenticate against the OpenID Provider; afterwards
the user is redirected to the previously provided redirect uri where the agent's
webserver is waiting for the response. The agent receives an authorization code
that is exchanged for the required token. `oidc-gen` is polling `oidc-agent` to get
the generated account configuration and finally save it.

#### Device Flow
The device flow is a flow specifically for devices with limited input
possibilities or without a web browser. Unfortunately, it is currently not supported by many
OpenID Providers.

To use the device flow the user has to call `oidc-gen` with the `--flow=device`
option. `oidc-gen` will print a verification url and an user code. If `qrencode`
is installed on the system, the verification url is also printed as a QR-Code.
The user must open the
given url using a second device and enter the given user code. Through polling
the agent will get a refresh token and `oidc-gen` the generated account
configuration.

If the OpenID Provider does not provide the device authorization endpoint in
their openid-configuration it has to provided manually using the `--dae`
option. For help on a specific provider check the [provider
documentation](../provider/provider.md)


### `--manual`
This option has to be used if a user wants to use a manually registered client.
`oidc-gen` will then not use dynamic client registration. Additional
metadata about the already registered client must be passed to `oidc-gen`
when beeing prompted or using command line arguments (where they are available).

### `--no-scheme`
This option can be used when the authorization code flow is performed. The `--no-scheme` option tells
`oidc-agent` that a custom uri scheme should not be used for redirection. Normally a custom uri scheme can be used to
redirect direct to (another) oidc-gen instance when performing the
authorization code flow instead of using a web server. However, the redirect to
oidc-gen requires a graphical desktop environment. If this is not present,
redirection with custom uri schemes can be disabled with this option.

This option can be used with `oidc-gen` or `oidc-agent`. When using it
with `oidc-gen` it will only disable custom uri schemes for that specific call;
when using it with `oidc-agent` it will disable custom uri schemes for all calls
to that `oidc-agent` instance.

### `--no-url-call`
When using the authorization code flow the user must authenticate against the
OpenID Provider using a webbrowser. To do this `oidc-gen` prints an
authorization url the user has to open. On default this url is automatically
opened in the default webbrowser (using `xdg-open`). One can disable this
behavior with the `--no-url-call` option. When this option is passed
`oidc-gen` will not automatically open the authorization url. The user then
has to manually copy it to his webbrowser.

### `--no-webserver`
This option can be used when the authorization code flow is performed. On default a small
webserver is started by `oidc-agent` to be able to catch the redirect and
complete the authorization code flow. The `--no-webserver` option tells
`oidc-agent` that no webserver should be started. The authorization code
flow can still be completed. Either by using a redirect uri that follows the
custom redirect uri scheme `edu.kit.data.oidc-agent:/<path>` - this will
directly redirect to oidc-gen, or by copying the url the browser would normally
redirect to and pass it to `oidc-gen --codeExchange`.

This option can be used with `oidc-gen` or `oidc-agent`. When using it
with `oidc-gen` it will only disable the webserver for that specific call;
when using it with `oidc-agent` it will disable the webserver for all calls
to that `oidc-agent` instance.

### `--only-at`
The `--only-at` option of `oidc-gen` can be used to obtain an access token without creating an account configuration.
You still have to provide a valid client configuration. There are several ways of doing so. The option can be combined with the different ways of using `oidc-gen`, but it will not work with dynamic client registration.
The following is a short overview:
- `oidc-gen --only-at -m` Manually provide the needed information (with prompting)
- `oidc-gen --only-at -f <filepath>` Manually provide the needed information by passing the path to a json file with the client information.
- `oidc-gen --only-at` Use a public client defined in the `pubclients.conf` file.

Notes:
- It's possible to overwrite some or all of the passed values with command line options.
- You always have to provide the issuer url and the scopes to be used (but can do so in the passed file).
- When using a public client that does not have a client secret you must pass the `--pub` option.

Here are three examples how a user can obtain the access token and store it in an environment variable without providing any other information:
```
export AT=`oidc-gen --only-at --iss=<issuer_url> --scope-max --prompt=none` # requires that a public client for <issuer_url> is listed in pubclients.conf
export AT=`oidc-gen --only-at --iss=<issuer_url> --client-id=<client_id> --client-secret=<client_secret> --redirect-url="http://localhost:8080" --scope=profile --prompt=none`
export AT=`oidc-gen --only-at -f<filepath> --prompt=none`
```
In the last call `<filepath>` points to file with the following content:
```
{
  "issuer_url": "https://example.com",
  "client_id": "clientid",
  "client_secret": "clientsecret",
  "scope": "openid profile email",
  "redirect_uris": [
    "http://localhost:8080",
    "http://localhost:34170",
    "http://localhost:4242"
  ]
}
```

Note that the `--only-at` option can be used with any flow.

### `--print`
Using this option `oidc-gen` will read the specified file and print out the
decrypted content. You can either pass an account shortname to print out the
decrypted account configuration or an absolute filepath (needed if you want to
decrypt client configuration files or other files encrypted by `oidc-gen`).

### `--prompt`
This option can be used to change how `oidc-gen` prompts the user for information. There are different options available. Allowed values are `cli`, `gui`, and `none`. The default is `cli`. This is the normal mode as it also was before version 4.0.0. The suer is prompted for information on the command line. When selecting `gui`, `oidc-gen` will use graphical pop-up prompts. This requires `oidc-agent-prompt` to be installed. When `none` is selected, `oidc-gen` will not prompt for any information, which requires that the needed information is passed through command line options.

Changing the prompt mode to `gui` will also change the password prompt mode to
`gui` (see [`--pw-prompt`](#pw-prompt)). Changing it to `none`, will not change
the password prompt mode, because this cannot set to `none`.

### `--pub`
When this option is provided, `oidc-gen` will use a public client. If
`--manual` is not provided, normally a client would be registered
dynamically. However, with the `--pub` option, a preregistered public client
is used. Preregistered public clients are listed in
`/etc/oidc-agent/pubclients.config`. If the `--manual` option is
specified this allows usage of a public client that was registered manually (the
`client_secret` parameter will be optional).

This option is also required to update an account configuration that uses a public client.

### `--pw-cmd`
By default `oidc-gen` will prompt the user for an encryption password when
it needs to encrypt or decrypt an account configuration.
The option `--pw-cmd` can be used to provide a command that will print the
needed encryption password to `stdout`. Then `oidc-gen` can obtain the
password from that command instead of prompting the user.

### `--pw-env`
By default `oidc-gen` will prompt the user for an encryption password when
it needs to encrypt or decrypt an account configuration.
The option `--pw-env` can be used to provide the encryption password via an
environment variable. The name of the environment variable can be passed to
`--pw-env`. If this option is used without an argument the
encryption password is read from the environment variable `OIDC_ENCRYPTION_PW`.

### `--pw-file`
By default `oidc-gen` will prompt the user for an encryption password when
it needs to encrypt or decrypt an account configuration.
The option `--pw-file` can be used to provide the path to a file that contains the
needed encryption password. Then `oidc-gen` can obtain the
password from that file.

### `--pw-prompt`
This option can be used to change how `oidc-gen` prompts the user for the
encryption password. Possible values are `cli` and `gui`. The default is `cli`.
`gui` requires oidc-agent-prompt to be installed.

### `--reauthenticate`
This option can be used to update an existing account configuration file with a
new refresh token. Useful if - for some reason - the refresh token is not valid
anymore. One could also use `--manual` to update an existing account
configuration; however if no other information has to be changed the
`--reauthenticate` option is easier.

### `--rename`
This option can be used to rename an existing account configuration file. It is not enough to simply rename the file in the file system. One could also use `--manual` to update an existing account
configuration; however if no other information has to be changed the
`--rename` option is easier.

### `--seccomp`
Enables seccomp system call filtering. See [general seccomp
notes](../security/seccomp.md) for more details.

### `--update`
This option can be used to update the encryption and / or file format for a file
generated by oidc-gen. It will decrypt and re-encrypt the file content, therefore
updating encryption and file format to the newest version. This option can also
be used to encrypt plain text files, e.g. a client configuration that was
downloaded from the OpenID Provider - do not use it as a general file encryption
tool.
The passed parameter can be an absolute path or the name of a file placed in oidc-dir (e.g. an account configuration short name).


### `--at`
The `--at` option is used during dynamic client registration. If the
registration endpoint is protected and can only accessed with proper
authentication, the user has to pass the token used for authentication to the
`--at` option.

### `--aud`
The `--aud` option can be used to set the audience of obtained access tokens.
Protected resources should not accept a token if they are not listed
as audience. Therefore, this is a mechanism to restrict the usage of an access
token to certain resources.

The audience of individual access tokens can also be set with `oidc-token
--aud`.

See [`oidc-token --aud`](../oidc-token/options.md#aud) for more information.

### `--client-id`
The `--client-id` option can be used to set the client id that should be used.

### `--client-secret`
The `--client-secrete` option can be used to set the client secret that should be used.

### `--cnid`
The `--cnid` option can be used to set an additional client name identifier. This might be useful in the case a user has multiple machines that run `oidc-agent` and he configures new account configurations for each machine. However, they should have the same shortname on all machines. While this is possible, the clientname for all of these clients will be of the form `oidc-agent:<shortname>`. With the same shortname the clients cannot be distinguished easily in a web interface provided by the OpenID Provider. Most provider allow to access a list with authorized applications. If a user has an account configuration for `example` on two different machines, he will see the `oidc-agent:example` entry twice and cannot identify which entry belongs to which machine.
However, this is possible using the `--cnid` option. This option allows the
user to specify an additional component of the client name
`oidc-agent:<shortname>-<cnid>`. A user could use for example the hostname
of the machine. Then there are two different applications listed in the
provider's web interface and the clients can be
matched to the correct machine where that client is indeed used.

### `--dae`
The `--dae` option explicitly sets the `device authorization endpoint uri`. When performing the device flow `oidc-agent` has to send information to this endpoint. Usually oidc-agent can obtain this uri from the provider's configuration endpoint. However, if the provider does not publish its device authorization endpoint uri at its configuration endpoint, the user has to tell `oidc-agent` where the device authorization endpoint can be found. Therefore, the uri has to be passed to the `--dae` option. Check the documentation about [providers](../provider/provider.md) for information if you need this option with your provider.

### `--issuer`
The `--issuer` option can be used to set the issuer url that should be used.

### `--op-password`
The `--op-password` option can be used to set the user's password at the OpenID
provider. This option only applies when the password flow is used. Note that it
is not recommended to use the password flow in general; even more it is not
recommended to set the password from the command line. Please use prompting for
this.

### `--op-username`
The `--op-username` option can be used to set the user's username at the OpenID
provider. This option only applies when the password flow is used.

### `--port`
This option can be used to set redirect uris. Only the port must be provided and
it will result in a redirect uri of the form `http://localhost:<port>`.
This option is a short option for `--redirect-uri` (only the port has to be
provided).
Passing `--port=1234` is equivalent to passing
`--redirect-uri=http://localhost:1234`.

For more information see [-`-redirect-uri`](#redirect-uri).

### `--redirect-uri`
This option can be used to set the redirect uri to be used.
This applies to two cases:
- When the client was manually registered, the option can be used to pass the
    registered redirect uris.
- When the client will be registered dynamically, the option can be used to pass
    the redirect uris that should be registered.

On default `oidc-agent` will register multiple redirect uris when using dynamic client registration.
One redirect uri that
uses the custom uri scheme `edu.kit.data.oidc-agent:/<path>` and three
redirect uris to `localhost` using different port numbers. Two of these port
numbers are `4242` and `8080`; the third port number will be chosen
randomly. When starting the webserver `oidc-agent` will try all of these
ports, stopping when the first succeeds. We cannot make any guarantees on the
order in which these ports are tried.

This might be a problem in environments with restrictions to ports, e.g.
containers. In such environments it's useful to use the `--redirect-uri` or `--port` option to manually set the
port(s) that should be used (`oidc-agent` will register redirect uris with that port
numbers). By using these options one can pass
only ports that will be available in the restricted environment. Note that
`oidc-agent` still makes no guarantees about the order in which these ports
will be tried.

### `--rt`
This option can be used to pass a refresh token that should be used. Because
this will use the refresh flow this option implicitly sets `--flow=refresh`.

**Note:** Refresh tokens are bound to a specific client id. The provided refresh
token must be issued for the provided client id.

### `--rt-env`
Like `--rt` but reads the refresh token from an environment variable.
The name of the environment variable can be passed to
`--rt-env`. If this option is used without an argument the
refresh token is read from the environment variable `OIDC_REFRESH_TOKEN`.

### `--scope`
The `--scope` option can be used to set the scopes that should be used with this
account configuration. When using multiple scopes, provide a space separated
list.

### `--scope-all` and `--scope-max`
The `--scope-all` and `--scope-max` options can be used to set the scopes that
should be used with this account configuration to the maximum. I.e. this means
that all scopes supported by the provider will be used. When using a public
client all scopes available for that client will be used.
