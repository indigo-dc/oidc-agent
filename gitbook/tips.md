# Tips
Here we want to share some useful tips on how ```oidc-agent``` and the other
components can be used in your everyday work.

## Xsession Integration
See [Xsession Integration](configure.md#xsession-integration).

## oidc-token syntax
If you have to pass an access token to another application on the command line
you can substititue the token with ````oidc-token <shortname>` ```. E.g.
you can do an ```curl``` call with an OIDC token in the authorization header:
```
curl example.com -H 'Authorization: Bearer `oidc-token <shortname>`'
```
This syntax can be used with many applications and is quite useful.

## Obtaining More Information From oidc-token
As described under
[oidc-token](oidc-token.md#information-available-from-oidc-token) you can obtain
more infromation when calling ```oidc-token``` and not only the access token. If
you want to do this we recommand the ```--env``` option and call
```oidc-token``` the following way: ```eval `oidc-token --env <shortname>` ```.
This way the environment variables ```OIDC_AT```, ```OIDC_ISS```, and
```OIDC_EXP``` or populated with the correct values. 

A way that is **not** recommended, is to do multiple successive calls to ```oidc-token``` and only providind one of the ```--token```, ```--issuer```, ```--expires-at``` options on each call. 
This would make three independet token
requests to oidc-agent. This is not only inefficient but also does not gurante to
return correct results. It might happen that the token requested in the first
call is only valid for a very short time and not valid anymore when doing the
last request; in this case a new token will be requested that has a different
expiration time that does not relate to the token from the first call.

## Autoloading and Autounloading Account Configurations
since version ```2.3.0``` there is support for the so called ```autoload``` of
account configurations. If an application requests an access token for an
configuration that is not loaded, it can be automatically loaded. The user will
be prompted to enter the encryption password for this account configuration and
through that can decide if the account should be loaded or not. This means we
can do a call to ```oidc-token example``` and even if ```example``` is currently
not loaded, it will be loaded and an access token is returned.

The autoloading feature makes it quite easy to also use the autounload (limited
lifetime of account configuration). When starting ```oidc-agent``` with the
```--lifetime``` option you can specify for how long account configuration
should be loaded (the default is forever). However, we now can use a limit and
load account configuration only for e.g. 300 seconds. After that time the
account configuration will automatically be removed from the agent. But if an
application needs an access token for an account configuration it can be easy
loaded through the autoload feature.

This way the time sensitive information is kept in memory by ```oidc-agent```
can be limited without reducing usability much (the user does not always have to
run ```oidc-add```). Of course there are use cases where the autounload-autoload
combination is not useful, e.g. if a script runs periodically and needs an
access token and should run with really no user interaction.
