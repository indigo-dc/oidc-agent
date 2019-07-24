# Tips
Here we want to share some useful tips on how ```oidc-agent``` and the other
components can be used in your everyday work.

* [Xsession Integration](#xsession-integration)
* [Command Line Integration of oidc-token](#command-line-integration-of-oidc-token)
* [Obtaining More Information From oidc-token](#obtaining-more-information-from-oidc-token)
* [Autoloading and Autounloading Account Configurations](#autoloading-and-autounloading-account-configurations)
* [Running oidc-agent on a server](#running-oidc-agent-on-a-server)
* [Updating an Expired Refresh Token](#updating-an-expired-refresh-token)
* [Applications that run under another user](#applications-that-run-under-another-user)

## Xsession Integration
See [Xsession Integration](configure.md#xsession-integration).

## Agent Forwarding
See [Agent Forwarding](configure.md#agent-forwarding).

## Using oidc-token With an Issuer Instead of the Shortname
Instead of using ```oidc-token <shortname>``` you also can do ```oidc-token
<issuer_url>```. Usually the usage of the shortname is shorter than using the
whole issuer url. However, there are use cases where this option might be quite
useful. Generally it is a more universal way to obtain an access token for a
specific provider. While ```oidc-token mySuperFancyShortname``` might work on
your machine it can fail for other users because they do not have
```mySuperFancyShortname```. Using the issuer url 
- when writing scripts that are shared with other users, 
- opening issues that mention calls to ```oidc-token```, or 
- sharing other ```oidc-token``` related commands
makes it easier for other users to run the same commands without any changes.

## Command Line Integration of oidc-token
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
Since version ```2.3.0``` there is support for the so called ```autoload``` of
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

## Running oidc-agent on a server
`oidc-agent` can run on a remote server. However, if you are planning to do
this, you should check your use case, if this is really necessary. [Agent
forwarding](configure.md#agent-forwarding) can be used to access a local agent
on a remote server.

Generating a new account configuration file on a remote server can be more
difficult, because there is neither a webbrowser nor a desktop environment. But
because oidc-agent is designed for command line usage, it is still possible.
There are several ways of generating a new account configuration on a remote
server:
0. Generate it locally and copy it to the remote server
1. Using the password flow, which can be done entirely on the command line
2. Using the device flow, where a second device is used for the web-based
   authentication.
3. Using the authorization code flow with a 'manual redirect'

Option 1 and 2 are not supported by all providers. However, if the device flow
is supported by your provider, we recommend option 2.

When doing option 3 you should be aware of the following:
- Make sure that the agent uses the `--no-webserver` and `--no-scheme` options
or pass these options to `oidc-gen`
- Also add the `--no-url-call` option when calling `oidc-gen`
- Copy the printed authorization url and open it in a browser (local).
- Authenticate and authorize oidc-agent as usual
- You will be redirected to localhost. Because there is no webserver listening
your browser will display an error message.
- Copy the url to which you are redirected from the address bar of your browser
- Head over to the remote server and pass the copied url to `oidc-gen` in the
following call:
```
oidc-gen --code-exchange='<url>'
```

## Updating an Expired Refresh Token
If a refresh token expired the user has to reauthenticate to obtain a new valid
refresh token. Until version ```2.3.0``` this would require the user to use
```oidc-gen -m <shortname>```, which allows the user to change all data of
this account configuration (and therefore has to confirm all existing data).
Because the user only wants to reauthenticate to update the refresh token,
confirming that all other data should be unchanged annoying. Instead use
```oidc-gen --reauthenticate <shortname>```. This option will only start the
reauthentication and update the refresh token. Easier and faster.

## Applications that run under another user
On default only applications that run under the same user that also started the
agent can obtain tokens from it. Whens tarting the agent the `--with-group` option can be used to
allow other applications to also access the agent. This can be useful in cases where
applications must run under a specific user. 

The user first has to create a
group (e.g. named `oidc-agent`) and add himself and all other users that need
access to the agent to this group. It is the user's responsibility to manage
this group. Then he can pass the group name to the `--with-group` option to
allow all group members access to the agent. If the option is used without
providing a group name, the default is `oidc-agent`.
