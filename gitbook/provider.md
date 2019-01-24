# How to get an account configuration with ...
In this section we describe how to generate a working account configuration for
some of the supported OpenID Providers.

* [IAM (INDIGO/DEEP)](#iam-indigodeep)
* [Goggle](#google)
* [KIT](#kit)
* [B2Access](#b2access)
* [EGI](#egi-check-in)
* [HBP](#human-brain-project-hbp)
* [Elixir](#elixir)
* [Another provider](#a-provider-not-listed)

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
use the password flow or device flow instead; see [Password Flow](oidc-gen.md#password-flow) and [Device Flow](oidc-gen.md#device-flow).


## Google
Google does not support dynamic client registration, but there is a
preregistered public client.

### Quickstart
Example:
```
$ oidc-gen <shortname>
[...]
Issuer [https://accounts.google.com/]: 
Space delimited list of scopes [openid profile offline_access]: 
Registering Client ...
Dynamic client registration not supported by this issuer.
Try using a public client ...
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

A client can be registered manually at <https://console.developers.google.com/> There is documentation
on how to do this at
<https://developers.google.com/identity/protocols/OpenIDConnect> (just the first
section "Setting up OAuth 2.0").

After the client registration you can download the client configuration as a
json file. You can pass this file to oidc-gen using the ```-f``` flag. If you
don't do this you have to enter the configuration manually (you than have to
call oidc-gen with the ```-m``` flag).

When using the Device Flow with Google you have to call oidc-gen with the
```--flow=device``` option. Additionally for Google you have to provide the
device authorization endpoint using the ```--dae``` option. Note also that the
registered OIDC client has to be of type 'native'/'other' and not 'web
application'.

Example call for using the device flow with google:
```
oidc-gen google -m --flow=device --dae=https://accounts.google.com/o/oauth2/device/code
```

## KIT
The KIT OIDP supports dynamic client registration, but a special access token is
required as authorization. The easiest way is too use the preregistered public
client.

### Quickstart
Example:
```
$ oidc-gen <shortname>
[...]
Issuer [https://oidc.scc.kit.edu/auth/realms/kit/]:
Space delimited list of scopes [openid profile offline_access]:
Registering Client ...
The following error occured during dynamic client registration:
Policy 'Trusted Hosts' rejected request to client-registration service. Details: Host not trusted.
Try using a public client ...
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
To get an inital access token please contact [Matthias
Bonn](mailto:matthias.bonn@kit.edu).
The token can then be used as authorization through the ```--at``` option.


## B2ACCESS
B2ACCESS does not support dynamic client registration and you have to register a
client manually at <https://b2access.eudat.eu/>
<https://b2access-integration.fz-juelich.de/> or <https://unity.eudat-aai.fz-juelich.de/> (depending on the issuer url). There is documentation
on how to do this at <https://eudat.eu/services/userdoc/b2access-service-integration#UserDocumentation-B2ACCESSServiceIntegration-HowtoregisteranOAuthclient>

After the client registration call oidc-gen with the ```-m``` flag and enter the
required information. 
**Note:** For B2ACCESS 'client_id' is equivalent to the client 'username' and
'client_secret' to the client 'password'

## EGI Check-in
EGI Checki-in supports dynamic registration and a simple call to oidc-gen is therefore
enough to register a client and generate the account configuration.

Example:
```
$ oidc-gen <shortname>
[...]
Issuer [https://aai.egi.eu/oidc/]:
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

## Human Brain Project (HBP)
HBP supports dynamic registration, but has a protected registration endpoint. 
Therefore, you have to pass an inital access token to oidc-gen using the ```--at``` option. One way to obtain such an access token is using [WaTTS](https://watts.data.kit.edu/).

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

## Elixir
Elixir supports dynamic registration and a simple call to oidc-gen is therefore
enough to register a client and generate the account configuration.

However, the registered client will only have the 'openid' scope. If you need
additional scopes, you have to request them manually. Therefore follow 
[these instrucations](https://docs.google.com/document/d/1ihb0hH2YJqSCPZS0syVpvAOeQP1HTxdf_XMsZZLe_W0/)
starting at Step 2 Point 5.
After the scopes are updated you have to update the account configuration with
the correct scopes. Therefore call ```oidc-gen -m <short_name>``` and change the
scope value.

After client registration oidc-agent will use the authorization code flow to
obtain a refresh token and generate the account configuration. If you want to
use the device flow (which is supported by Elixir) you have to call oidc-gen
with the ```--flow=device``` option.

## A provider not listed
If your provider wasn't listed above, don't worry - oidc-agent should work with any OpenID Provider. Please
follow these steps.

### Try Dynamic Client Registration
If you already have a registered client you can see [Generate the
Account Configuration](#generate-the-account-configuration).

Dynamic client registration is not supported by all OpenID Providers, so it
might fail. Anyway try registering a client dynamically by calling oidc-gen and
provide the issuer url when beeing prompted.

If dynamic client registration is not supported, oidc-gen will tell you this.
In that case you have to register the client manually through the provider's web
interface and then go to [Generate the Account Configuration](#generate-the-account-configuration).

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

For information on the different client metadata needded for a manual client
registration see [Client Configuration
Values](oidc-gen.md#client-configuration-values).

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

If you experience any problems with expiring refresh tokens please contact us. 

Due to the security by design principle, oidc-agent is currently not able to
update a stored refresh token. To handle expiring and changing refresh tokens
while preserving privilege separation a major design change would be necessary.
We are willing to do so, if necessary.

If a refresh token expired, use ```oidc-gen -m <short_name>``` to reinitialize
the account configuration.
