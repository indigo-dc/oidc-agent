# oidc-gen
You can use oidc-gen to generate a new agent account configuration. 

```
$ oidc-gen --help
Usage: oidc-gen [OPTION...] [ACCOUNT_SHORTNAME]
oidc-gen -- A tool for generating oidc account configurations which can be used
by oidc-add

 Getting information:
  -c, --clients              Prints a list of available client configurations
  -l, --accounts             Prints a list of available account configurations.
                             Same as oidc-add -l
  -p, --print=FILE           Prints the decrypted content of FILE. FILE can be
                             an absolute path or the name of a file placed in
                             oidc-dir (e.g. an account configuration short
                             name)

 Generating a new account configuration:
      --at[=ACCESS_TOKEN]    An access token used for authorization if the
                             registration endpoint is protected
  -d, --delete               Delete configuration for the given account
  -f, --file=FILE            Reads the client configuration from FILE.
                             Implicitly sets -m
  -m, --manual               Does not use Dynamic Client Registration. Client
                             has to be manually registered beforehand

 Advanced:
      --cnid=CLIENTNAME_IDENTIFIER
                             Additional identifier used in the client name to
                             distinguish clients on different machines with the
                             same short name, e.g. the host name
      --cp[=CERT_PATH]       CERT_PATH is the path to a CA bundle file that
                             will be used with TLS communication
      --dae=ENDPOINT_URI     Use this uri as device authorization endpoint
      --no-seccomp           Disables seccomp system call filtering; allowing
                             all system calls. Use this option if you get an
                             'Bad system call' error and hand in a bug report.
      --no-url-call          Does not automatically open the authorization url
                             in a browser. Enables oidc-gen to use seccomp.
  -o, --output=OUTPUT_FILE   When using Dynamic Client Registration the
                             resulting client configuration will be stored in
                             OUTPUT_FILE instead of inside the oidc-agent
                             directory. Implicitly sets the -s option.
      --qr                   When using the device flow a QR-Code containing
                             the device uri is printed
      --qrt                  When using the device flow a QR-Code containing
                             the device uri is printed directly to the
                             terminal. Implicitly sets --qr
      --rt[=REFRESH_TOKEN]   Use the specified REFRESH_TOKEN with the refresh
                             flow instead of using another flow. Implicitly
                             sets --flow=refresh
  -s, --split-config         Use separate configuration files for the
                             registered client and the account configuration.
  -w, --flow=code|device|password|refresh
                             Specifies the OIDC flow to be used. Option can be
                             used multiple times to allow different flows and
                             express priority.

 Internal options:
      --codeExchangeRequest=REQUEST
                             Only for internal usage. Performs a code exchange
                             request with REQUEST
      --state=STATE          Only for internal usage. Uses STATE to get the
                             associated account config

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
## Client Registration
### Dynamic Client Registration - Manual Client Registration
oidc-agent requires a registered client for every OpenID Provider used. Most likely a user
does not have a already registered client and does not want to do it through a web interface. 
If the OpenID Provider supports dynamic registration, the agent can register a new client dynamically.
One big advantage of using dynamic registration is the fact that oidc-agent will
register the client with exactly the configuration it needs.
Dynamic Registration is the default option and running ```oidc-gen``` is enough.

If a user already has a client registered or the OpenID Provider does not support
dynamic client registration oidc-gen must be called with the ```-m``` option. oidc-gen will prompt the user for the relevant
information. If the user has a file with the client configuration information he can pass it to oidc-gen using the ```-f``` flag.
When registering a client manually be careful with the provided data. Check
the following section for the values important to oidc-agent.

### Client Configuration Values
To register an OIDC Client their a some specific configuration values. When
registering a client manually you might have to provide quite a number of these.
Also oidc-gen might prompt your for some when using dynamic client registration. If you
are not familiar with one of these values, please check this section.

When registering a client an OpenID Provider might be using default values for
some of these configurations so you might not need all of them.

#### Scope
OpenID Connect Clients use scope values to specify what access privileges are being requested for Access Tokens.
Required scopes for oidc-agent are: 'openid' and 'offline_access'. Additional scopes can be
registered if needed.

When using dynamic registration the user will be prompted to enter scopes that
will be registered with that client. The keyword ```max``` can be used to
request all supported scopes.

Example Scope: openid profile offline_access

#### Redirect Uri
The Redirect Uri is used during the Authorization Code Flow. The Redirect Uri must
be of the following scheme: ```http://localhost:<port>``` where ```<port>``` should be an
available port. It is also possible to specify an additional path, e.g.
```http://localhost:8080/redirect```, but this is not required. It is important that this port is not used when generating the
account configuration with oidc-gen. Multiple Redirect Uris can be registered to
have a backup port if the first one may be already in use. 

Example Redirect Uris: http://localhost:8080 http://localhost:2912

#### Response Type
The following response types must be registered:
- 'token' when using the Password Flow
- 'code' when using the Authorization Code Flow

#### Grant Types
The following grant types must be registered:
- 'refresh_token' if available
- 'authorization_code' when using the Authorization Code Flow 
- 'password' when using the Password Flow
- 'urn:ietf:params:oauth:grant-type:device_code' when using the Device Flow

## Choose a Flow
Depending on the OpenID Provider you have multiple OpenID/OAuth2 Flows to choose
from. oidc-agent uses the Refresh Flow to obtain additional access token.
Therefore a refresh token is required which can be obtained in multiple ways:

### Out Of Band
If you obtained a refresh token out of band you can directly provide it to
oidc-gen when beeing prompted. Optionally you can additionally set
--flow=refresh when calling oidc-gen, but it is not required.

**Note:** Refresh token are bound to a specific client id. The provided refresh
token has to be issued for the provided client id.

Obtaining a valid refresh token for the specific client id is out of scope of
this documentation. We recommend one of the following flows.

### Password Flow
The password flow is not supported by most providers. One provider that supports the password flow is INDIGO IAM.
The password flow can be done using only the
command line. The credentials for the OpenID Provider have to be provided to
oidc-gen. The credentials are only used to obtain the refresh token and are not stored. 
However, there are alternatives flows that do not reveal your credentials to
oidc-agent.

Using IAM the password grant type is not supported in dynamic registration. The client is registered without it
and you have to contact the provider to update the client config manually. After that is
done, you can run oidc-gen again with the same shortname. oidc-gen should find a temp file and continue the account configuration generation. If the temp file was not found (e.g. after a reboot) you can specify the saved client config file to oidc-gen using ```oidc-gen -f <filepath>```
and finish the account configuration. Afterwards the config is added to oidc-agent 
and can be used by oidc-add normally to add and remove the account configuration from the agent.

### Authorization Code Flow
The code flow is the most widely used and is therefore supported by any OpenID
Provider and does not reveal user credentials to oidc-agent. However, it
requires a browser on the system running oidc-agent and oidc-gen. If you don't
have a browser on that system or don't want to use it you can use the Device
Flow, if supported by the provider. To use the Authorization Code Flow at
least one redirect uri has to be provided (see [Redirect Uri](#redirect-uri)). 
The redirect uri must be of the scheme ```http://localhost:<port>``` It is 
recommend to use a port which is very unlikely to be used by any other 
application (during the account generation process). Additionally multiple 
redirect uris can be provided.

When starting the account generation process oidc-agent will try to open a
webserver on the specified ports. If one port fails the next one is tried. After
a successful startup oidc-gen will receive an authorization URI. When calling
this URI the user has to authenticate against the OpenID Provider; afterwards
the user is redirected to the previously provided redirect uri where the agent's
webserver is waiting for the response. The agent receives an authorization code
that is exchanged for the required token. oidc-gen is polling oidc-agent to get
the generated account configuration and finally save it.

### Device Flow
The device flow is a flow specifically for devices with limited input
possibilities or without a web browser. Unfortunately, it is currently not supported by many
OpenID Providers.

To use the device flow the user has to call oidc-gen with the ```--flow=device``` option.
oidc-gen will print a verification url and an user code. (If the user includes
the ```--qr``` option and ```qrencode``` is installed on the system, a QR-Code
containing the verification url is printed.) The user must go to the
given url using a second device and enter the given user code. Through polling
the agent will get a refresh_token and oidc-gen the generated account
configuration.

If the OpenID Provider does not provide the device authorization endpoint in
their openid-coniguration it has to provided manually using the ```--dae```
option.

### The ```--flow``` Flag
The ```--flow``` flag can be used to enforce usage of a specific flow or to
prioritise a flow over another.
oidc-agent will try all flows in the following order until one succeeds: 
1. Refresh Flow
2. Password Flow
3. Authorization Code Flow
4. Device Flow

For every flow there is at least one field that must be present (except device flow). The Refresh Flow
requires a refresh token; the Password Flow requieres username and password; the
Authorization Code Flow requires at least one redirect uris. oidc-agent will
detect if these values are present or not and will try the corresponding flow.
E.g. if no refresh token is given, the refresh flow won't be tried. But if a
refresh token is given, but not valid the refresh flow will fail and the next
flow will be tried.

To prevent that, you can enforce usage of a specific flow by using the ```--flow``` 
flag. Possible values are: 'refresh', 'password' and 'code'. 
The flag can also be used if multiple flows should be tried, but in a different
order than the default one. To do so provide the option multiple times with one
value per option in the desired order.

## Edit an existing account configuration
If you want to edit an existing configuration, you can do so by running oidc-gen
and providing the short name for this configuration.

## oidc-gen and oidc-add
oidc-gen will also add the generated configuration to the agent. So you don't
have to run oidc-add afterwards. However, if you want to load an existing
configuration don't use oidc-gen for it; oidc-add is your friend.


