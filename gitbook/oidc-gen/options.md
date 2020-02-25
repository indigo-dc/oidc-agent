## Detailed Information About All Options

* [`--clients`](#clients)
* [`--accounts`](#accounts)
* [`--print`](#print)
* [`--at`](#at)
* [`--delete`](#delete)
* [`--file`](#file)
* [`--manual`](#manual)
* [`--reauthenticate`](#reauthenticate)
* [`--aud`](#aud)
* [`--cnid`](#cnid)
* [`--codeExchange`](#codeExchange)
* [`--cp`](#cp)
* [`--dae`](#dae)
* [`--no-url-call`](#no-url-call)
* [`--no-webserver`](#no-webserver)
* [`--output`](#output)
* [`--port`](#port)
* [`--pub`](#pub)
* [`--pw-cmd`](#pw-cmd)
* [`--qr`](#qr)
* [`--qrt`](#qrt)
* [`--rt`](#rt)
* [`--split-config`](#split-config)
* [`--seccomp`](#seccomp)
* [`--update`](#update)
* [`--flow`](#flow)

### `--clients`
Using this option `oidc-gen` will print out a list of all
client configurations saved on the system. Before version 2.0.0 oidc-gen did
save the client configuration and the account configuration into separate files.
This is still possible using the `--split-config` option.
The `--clients` option will only list files if you:
- used dynamic client registration with `oidc-gen` before version 2.0.0
- used dynamic client registration with `oidc-gen --split-config`
- manually placed a file ending with `.clientconfig` in the oidc-agent
  directory.

If you want manually registered clients listed here, put the client
configuration in a file placed in the oidc-agent directory with a filename
ending with `.clientconfig`. Optionally it can be encrypted by passing the
absolute filepath to `oidc-gen --update`.

### `--accounts`
Using this option `oidc-gen` will print out a list of all
configured account configurations. Configured means that they are saved on the
system and can be loaded with `oidc-add`; it does not mean that they are
currently loaded.
This option is the same as `oidc-add --list`. To show a list of the
accounts that are currently loaded use `oidc-add --loaded`.

### `--print`
Using this option `oidc-gen` will read the specified file and print out the
decrypted content. You can either pass an account shortname to print out the
decrypted account configuration or an absolute filepath (needed if you want to
decrypt client configuration files or other files encrypted by `oidc-gen`).

### `--at`
The `--at` option is used during dynamic client registration. If the
registration endpoint is protected and can only accessed with proper
authentication, the user has to pass the token used for authentication to the
`--at` option.

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

### `--manual`
This option has to be used if a user wants to use a manually registered client.
`oidc-gen` will then not use dynamic client registration. Additional
metadata about the already registered client must be passed to `oidc-gen`
when beeing prompted or using command line arguments (where they are available).

### `--reauthenticate`
This option can be used to update an existing account configuration file with a
new refresh token. Useful if - for some reason - the refresh token is not valid
anymore. One could also use `--manual` to update an existing account
configuration; however if no other information has to be changed the
`--reauthenticate` option is easier.

### `--aud`
The `--aud` option can be used to set the audience of obtained access tokens.
Protected resources should not accept a token if they are not listed
as audience. Therefore, this is a mechanism to restrict the usage of an access
token to certain resources.

The audience of individual access tokens can also be set with `oidc-token
--aud`.

See [`oidc-token --aud`](../oidc-token/options.md#aud) for more information.

### `--cnid`
The `--cnid` option can be used to set an additional client name identifier. This might be useful in the case a user has multiple machines that run `oidc-agent` and he configures new account configurations for each machine. However, they should have the same shortname on all machines. While this is possible, the clientname for all of these clients will be of the form `oidc-agent:<shortname>`. With the same shortname the clients cannot be distinguished easily in a web interface provided by the OpenID Provider. Most provider allow to access a list with authorized applications. If a user has an account configuration for `example` on two different machines, he will see the `oidc-agent:example` entry twice and cannot identify which entry belongs to which machine.
However, this is possible using the `--cnid` option. This option allows the
user to specify an additional component of the client name
`oidc-agent:<shortname>-<cnid>`. A user could use for example the hostname
of the machine. Then there are two different applications listed  in the
provider's web interface and the clients can be
matched to the correct machine where that client is indeed used.

### `--codeExchange`
When using the authorization code flow the user has to authenticate against the
OpenID Provider in a web browser and is then redirected back to the application.
To be able to catch that redirect `oidc-agent` usually starts a small webserver. If something goes wrong during the redirect (because the web server crashed or no web server was used (`--no-webserver`)) the user can still finish the account configuration generation process. In such a case the suer must copy the url he is redirected to from its browser and pass it to `oidc-gen --codeExchange`. Then oidc-gen should be able to obtain and save the final account configuration.

Note that while this option also works for `edu.kit.data.oid-cagent:/`
redirect uris, it might not be possible to obtain the uri the user is redirected
to from the browser.

### `--cp`
This option allows the user to change the CA bundle file that is used to verify
SSL/TLS certificates. A user must use this option when `oidc-gen` cannot
automatically find a proper CA bundle file (e.g. non default location). A user
can also use this option to provide a more restricted bundle file that only
contains the certificates needed for a specific provider.

### `--dae`
The `--dae` option explicitly sets the `device authorization endpoint uri`. When performing the device flow `oidc-agent` has to send information to this endpoint. Usually oidc-agent can obtain this uri from the provider's configuration endpoint. However, if the provider does not publish its device authorization endpoint uri at its configuration endpoint, the user has to tell `oidc-agent` where the device authorization endpoint can be found. Therefore, the uri has to be passed to the `--dae` option. Check the documentation about [providers](../provider/provider.md) for information if you need this option with your provider.

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

### `--no-scheme
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

### `--output`
This option only applies when dynamic client registration is used. The passed
parameter is the absolute filepath where the client configuration should be
stored. On default the client configuration is stored along the whole account
configuration in a single file, if the `--split-config` option is passed,
client and account configurations are stored in separated files, but both are
placed in the oidc-agent directory. To change the location of the client
configuration file this `--output` option can be used. Therefore, it
implicitly sets the `--split-config` option. The location of the account
configuration file cannot be changed.

### `--port`
This option only applies when dynamic client registration is used. On default
`oidc-agent` will register multiple redirect uris. One redirect uri that
uses the custom uri scheme `edu.kit.data.oidc-agent:/<path>` and three
redirect uris to `localhost` using different port numbers. Two of these port
numbers are `4242` and `8080`; the third port number will be chosen
randomly. When starting the webserver `oidc-agent` will try all of these
ports, stopping when the first succeeds. We cannot make any guarantees on the
order in which these ports are tried.

This might be a problem in environments with restrictions to ports, e.g.
containers. Therefore, the `--port` option can be used to manually set the
port(s) that should be used (`oidc-agent` will register redirect uris with that port
numbers). Multiple ports can be provided by using the option multiple times,
e.g. `oidc-gen --port=1234 --port=5678`. By using this option one can pass
only ports that will be available in the restricted environment. Note that
`oidc-agent` still makes no guarantees about the order in which these ports
will be tried.

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

### `--qr`
This option only applies when the device flow is used. On default the
verification url is printed to the terminal and the user has to type this url in
his second device into his browser. Using an QR-Code the user can instead of
typing the url, scan the QR-Code and visit the encoded verification url.

The `--qr` option will display the QR-Code as an image (good quality). This requires `qrcode` to be installed.

### `--qrt`
This option only applies when the device flow is used. On default the
verification url is printed to the terminal and the user has to type this url in
his second device into his browser. Using an QR-Code the user can instead of
typing the url, scan the QR-Code and visit the encoded verification url.

The `--qrt` option will print the QR-Code with ASCII characters in the terminal. While this might be more integrated in the terminal than the `--qr` option, the QR-Code resolution is also much lower and not all QR-Code scanners will scan this code correctly. This requires `qrcode` to be installed.

### `--rt`
This option can be used to pass a refresh token that should be used. Because
this will use the refresh flow this option implicitly sets `--flow=refresh`.

**Note:** Refresh tokens are bound to a specific client id. The provided refresh
token must be issued for the provided client id.

### `--split-config`
This option only applies when using dynamic client registration. On default
account configuration metadata and metadata specific to the registered client
are saved in one combined account configuration file.
To split these configurations and save the client configuration in a separate
file (behavior before version 2.0.0) use the `--split-config` option.

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
The passed parameter can be an absolute path or the name of a  file placed in oidc-dir (e.g. an account configuration short name).

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
option. `oidc-gen` will print a verification url and an user code. (If the user includes
the `--qr` option and `qrencode` is installed on the system, a QR-Code
containing the verification url is printed.) The user must open the
given url using a second device and enter the given user code. Through polling
the agent will get a refresh token and `oidc-gen` the generated account
configuration.

If the OpenID Provider does not provide the device authorization endpoint in
their openid-configuration it has to provided manually using the `--dae`
option. For help on a specific provider check the [provider
documentation](../provider/provider.md)


