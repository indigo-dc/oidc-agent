# How to Get an Account Configuration With ...
In this section we describe how to generate a working account configuration for
some of the supported OpenID Providers.

* [B2Access](#b2access)
<!-- * [EduTeams](#eduteams) -->
* [EGI](#egi-check-in)
* [Elixir](#elixir)
* [Google](#google)
* [HBP](#human-brain-project-hbp)
* [HDF](#helmholtz-data-federation-hdf)
* [IAM (INDIGO/DEEP)](#iam-indigodeep)
* [KIT](#kit)
* [Another provider](#a-provider-not-listed)

If you have to register a client manually check the [Client Configuration
Values](#client-configuration-values) section.

## B2ACCESS
B2ACCESS does not support dynamic client registration and you have to register a
client manually at <https://b2access.eudat.eu/>
<https://b2access-integration.fz-juelich.de/> or <https://unity.eudat-aai.fz-juelich.de/> (depending on the issuer url). There is documentation
on how to do this at <https://eudat.eu/services/userdoc/b2access-service-integration#UserDocumentation-B2ACCESSServiceIntegration-HowtoregisteranOAuthclient>

After the client registration call oidc-gen with the ```-m``` flag and enter the
required information. 

**Note:** For B2ACCESS `client_id` is equivalent to the client 'username' and
`client_secret` to the client 'password'

<!-- ## EduTeams -->
<!-- EduTeams does not support dymanic client registration, but there is a -->
<!-- preregistered public client. -->
<!--  -->
<!-- Example: -->
<!-- ``` -->
<!-- $ oidc-gen --pub <shortname> -->
<!-- [...] -->
<!-- Issuer [https://proxy.demo.eduteams.org/]: -->
<!-- Space delimited list of scopes [openid profile offline_access]: -->
<!-- Generating account configuration ... -->
<!-- accepted -->
<!-- To continue and approve the registered client visit the following URL in a Browser of your choice: -->
<!-- https://[...] -->
<!-- [...] -->
<!-- success -->
<!-- The generated account config was successfully added to oidc-agent. You don't have to run oidc-add. -->
<!-- Enter encryption password for account configuration '<shortname>': -->
<!-- Confirm encryption Password: -->
<!-- ``` -->

## EGI Check-in
EGI Checki-in supports dynamic registration, but dynamically registered clients
will not have any scopes. Therefore users have to either register a client
manually or use a preregistered public client (recommended).

Example:
```
$ oidc-gen --pub <shortname>
[...]
Issuer [https://aai.egi.eu/oidc/]:
Space delimited list of scopes [openid profile offline_access]:
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.
Enter encryption password for account configuration '<shortname>':
Confirm encryption Password:
```

### Advanced options
If you register a client manually you have the option to disable 'Refresh tokens
for this client are reused'. If you disable this option each refresh token can
  only be used once. Therefore, a new refresh token will be issued after each
  refresh flow (whenever a new access token is issued). When the refresh token
  changes oidc-agent has to update the client configuration file and therefore
  needs the encryption password. Because with rotating refresh tokens, this will
  happen quite often it is recommended to allow oidc-agent to keep the password
  in memory by specifing the ```--pw-store``` option when loading the account
  configuration with ```oidc-add```. 
## Elixir
Elixir supports dynamic registration, but dynamically registered clients
will not have any scopes. Therefore users have to either register a client
manually (and get approval for the needed scopes) or use a preregistered public client (recommended).

Example:
```
$ oidc-gen --pub <shortname>
[...]
Issuer [https://login.elixir-czech.org/oidc/]:
Space delimited list of scopes [openid profile offline_access]:
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.
Enter encryption password for account configuration '<shortname>':
Confirm encryption Password:
```

### Advanced options

#### Manual Client Registration
If you register a client manually, please see https://docs.google.com/document/d/1vOyW4dLVozy7oQvINYxHheVaLvwNsvvghbiKTLg7RbY/

#### Device Flow
To use the device flow with Elixir, the client has to have the device grant type
registered. This is the case for our public client, however, it might most
likely not be the case for a manually registered client.
To use the deivce flow instead the authorization code flow with the
preregistered public client include the ```--flow=device --pub``` options.


## Google
Google does not support dynamic client registration, but there is a
preregistered public client so that account configuration generation is as easy
as with dynamic client registration.

### Quickstart
Example:
```
$ oidc-gen --pub <shortname>
[...]
Issuer [https://accounts.google.com/]: 
Space delimited list of scopes [openid profile offline_access]: 
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.

Enter encryption password for account configuration '<shortname>': 
Confirm encryption Password: 
```

### Advanced options

#### Manual Client registration
A client can be registered manually at <https://console.developers.google.com/> There is documentation
on how to do this at
<https://developers.google.com/identity/protocols/OpenIDConnect> (just the first
section "Setting up OAuth 2.0").

After the client registration you can download the client configuration as a
json file. You can pass this file to ```oidc-gen``` using the ```-f``` flag. If you
don't do this you have to enter the configuration manually (you than have to
call ```oidc-gen``` with the ```-m``` flag).

#### Device Flow
When using the Device Flow with Google you have to call oidc-gen with the
```--flow=device``` option. Additionally for Google you have to provide the
device authorization endpoint using the ```--dae``` option. Note also that the
registered OIDC client has to be of type `native`/`other` and not `web
application`.

Example call for using the device flow with Google:
```
oidc-gen google -m --flow=device --dae=https://accounts.google.com/o/oauth2/device/code
oidc-gen google --pub --flow=device --dae=https://accounts.google.com/o/oauth2/device/code
```

## Human Brain Project (HBP)
HBP supports dynamic registration, but has a protected registration endpoint. 
Therefore, a user has to be a member of the Human Brain Project and has to pass an inital access token to oidc-gen using the ```--at``` option. One way to obtain such an access token is using [WaTTS](https://watts.data.kit.edu/).

Example:
```
$ oidc-gen <shortname> --at=<access_token>
[...]
Issuer [https://services.humanbrainproject.eu/oidc/]:
Space delimited list of scopes [openid profile offline_access]:
Registering Client ...
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.
Enter encryption password for account configuration '<shortname>':
Confirm encryption Password:
```

Alternatively it is also possible to use a preregistered public client by using
the ```--pub``` option (```--at``` is not required in that case).

## Helmholtz Data Federation (HDF)
HDF does not support dynamic client registration and you have to register a
client manually: 
- Make sure you don’t have an active login in unity and visit the /home endpoint (i.e. https://unity.helmholtz-data-federation.de/home or https://login.helmholtz-data-federation.de/home)
- Click “Register a new account” on the top right
- Specify the required information and note that “User name” is your `client_id` and “Password credential” is your `client_secret`.

Note also that you have to enter at least one valid redirect uri, even if they
are not mandated by HDF (see [Client Configuration Values](#redirect-uri) for
more information).

After the client is registered, call oidc-gen with the ```-m``` flag and enter the
required information. 

## IAM (INDIGO/DEEP)
IAM supports dynamic registration and a simple call to oidc-gen is therefore
enough to register a client and generate the account configuration.

### Quickstart
Example:
```
$ oidc-gen <shortname>
[...]
Issuer [https://iam-test.indigo-datacloud.eu/]:
Space delimited list of scopes [openid profile offline_access]:
Registering Client ...
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.

Enter encryption password for account configuration '<shortname>':
Confirm encryption Password:
```

### Advanced options
Instead of using the authorization code flow one could also
use the password flow or device flow instead.

#### Password Flow
Using IAM the password grant type is not supported in dynamic client registration. The client is registered without it
and you have to contact the provider to update the client config manually. After that is
done, you can run oidc-gen again with the same shortname. oidc-gen should find a temp file and continue the account configuration generation. Afterwards the config is added to oidc-agent 
and can be used by oidc-add normally to add and remove the account configuration from the agent.
You have to provide the ```--flow=password``` option to all calls to ```oidc-gen```.

#### Device Flow
Because the current IAM version does not advertise support for the device flow,
the user have to specifically tell it to oidc-gen.
The following options have to be included for INDIGO / DEEP, resp.:
```
--flow=device --dae=https://iam-test.indigo-datacloud.eu/devicecode
--flow=device --dae=https://iam.deep-hybrid-datacloud.eu/devicecode
```

## KIT
The KIT OIDP supports dynamic client registration, but a special access token is
required as authorization. The easiest way is too use the preregistered public
client.

### Quickstart
Example:
```
$ oidc-gen --pub <shortname>
[...]
Issuer [https://oidc.scc.kit.edu/auth/realms/kit/]:
Space delimited list of scopes [openid profile offline_access]:
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.
Enter encryption password for account configuration '<shortname>':
Confirm encryption Password:
```

The KIT OpenID Provider issues a new refresh token when the current refresh
token was used in the refresh flow (whenever a new access token is issued). When the refresh token
  changes oidc-agent has to update the client configuration file and therefore
  needs the encryption password. Because with rotating refresh tokens, this will
  happen quite often it is recommended to allow oidc-agent to keep the password
  in memory by specifing the ```--pw-store``` option when loading the account
  configuration with ```oidc-add```.

### Advanced options
To get an inital access token please contact the [provider](https://www.scc.kit.edu/dienste/openid-connect.php).
The token can then be used as authorization through the ```--at``` option.

## A provider not listed
If your provider was not listed above, do not worry - oidc-agent should work with any OpenID Provider. Please
follow these steps.

### Try Dynamic Client Registration
If you already have a registered client you can see [Generate the
Account Configuration](#generate-the-account-configuration).

Dynamic client registration is not supported by all OpenID Providers, so it
might fail. Anyway, try registering a client dynamically by calling oidc-gen and
providing the issuer url when beeing prompted.

If dynamic client registration is not supported, oidc-gen will tell you this.
In that case you have to register the client manually through the provider's web
interface (see [Client Configuration Values](#client-configuration-values) for help with manual client registration) and then go to [Generate the Account Configuration](#generate-the-account-configuration).

Some providers have a protected registration endpoint which is not public. If so
oidc-agent needs an inital access token for authorization at the endpoint.
Please call oidc-gen with the ```--at``` option to pass the access token to
oidc-gen.

When the client was successfully registered the account configuration should be
generated automatically and you should be able to save and use it.

### Generate the Account Configuration
If you registered a client manually call oidc-gen with the ```-m``` flag or if
you have a file containing the json formatted client configuration pass it to
oidc-gen with the ```-f``` flag.

After entering the required information oidc-agent should be able to generate
the account configuration which is then usable.

### Still no Success?
If you still were not be able to get oidc-agent working with that provider,
please contact the provider or us at <https://github.com/indigo-dc/oidc-agent/issues>. We will
try to figure out if the problem is with oidc-agent or the provider.

## Known Issues
### Expiring Refresh Tokens
oidc-agent assumes that refresh tokens do not expire. But some providers might
use refresh tokens that expire after a certain time or when they are not used
for a specific time. To prevent the latter use oidc-agent / oidc-token regularly
(you also can use a cron job). 

oidc-agent is able to
update a stored refresh token. However, therefore it has to receive a new
refresh token from the provider. If a refresh token expired (e.g. because the token was used within the lifetime of that token), use ```oidc-gen --reauthenticate <short_name>``` to reauthenticate and update the refresh token.

# Client Configuration Values
 When
registering a client manually you might have to provide quite a number of
specific configuration values. And even when using dynamic client registration
```oidc-gen``` prompts you for some values. If you
are not familiar with one of these values, please check this section.

When registering a client an OpenID Provider might be using default values for
some of these configurations so you might not have to provide all of them.

## Scope
OpenID Connect clients use scope values to specify what access privileges are being requested for access tokens.
Required scopes for oidc-agent are: `openid` and `offline_access`. Additional scopes can be
registered if needed. Most likely you also want to register at least the
`profile` scope.

When using dynamic client registration the user will be prompted to enter scopes that
will be registered with that client. The keyword ```max``` can be used to
request all supported scopes.

Example Scope: ```openid profile offline_access```

## Redirect Uri
The Redirect Uri is used during the Authorization Code Flow. The Redirect Uri must
be of the following scheme: ```http://localhost:<port>``` where ```<port>``` should be an
available port. It is also possible to specify an additional path, e.g.
```http://localhost:8080/redirect```, but this is not required. It is important that this port is not used when generating the
account configuration with oidc-gen. Multiple Redirect Uris can be registered to
have a backup port if the first one may be already in use. 
```oidc-gen``` also supports a custom redirect scheme, that can be used to
redirect directly to oidc-gen. In that case the redirect uri has to be of the
form ```edu.kit.data.oidc-agent:/<path>```.

We recommend registering the following redirect uris:
 - ```http://localhost:4242```
 - ```http://localhost:8080```
 - ```http://localhost:43985```
 - ```edu.kit.data.oidc-agent:/redirect```

Note: Only pass the ```edu.kit.data.oidc-agent:/redirect``` uri to oidc-gen, if
you wish to directly redirect to oidc-gen without using a webserver started by
oidc-agent.

## Response Type
The following response types must be registered:
- 'token' when using the Password Flow (see also
  [flow](oidc-gen.md#password-flow)) 
- 'code' when using the Authorization Code Flow (see also [flow](oidc-gen.md#authorization-code-flow))

## Grant Types
The following grant types must be registered:
- 'refresh_token' if available
- 'authorization_code' when using the Authorization Code Flow  (see also [flow](oidc-gen.md#authorization-code-flow))
- 'password' when using the Password Flow (see also
  [flow](oidc-gen.md#password-flow))
- 'urn:ietf:params:oauth:grant-type:device_code' when using the Device Flow (see also [flow](oidc-gen.md#device-flow))



