# Tips

Here we want to share some useful tips on how `oidc-agent` and the other components can be used in your everyday work.

* [Xsession Integration](#xsession-integration)
* [Command Line Integration of oidc-token](#command-line-integration-of-oidc-token)
* [Obtaining More Information From oidc-token](#obtaining-more-information-from-oidc-token)
* [Autoloading and Autounloading Account Configurations](#autoloading-and-autounloading-account-configurations)
* [Obtaining access tokens on a server](#obtaining-access-tokens-on-a-server)
* [Updating an Expired Refresh Token](#updating-an-expired-refresh-token)
* [Applications that run under another user](#applications-that-run-under-another-user)
* [Non-interactive oidc-gen](#non-interactive-oidc-gen)

## Xsession Integration

See [Xsession Integration](configuration/integration.md#xsession-integration).

## Agent Forwarding

See [Agent Forwarding](configuration/forwarding.md).

## Using oidc-token With an Issuer Instead of the Shortname

Instead of using `oidc-token <shortname>` you also can do `oidc-token
<issuer_url>`. Usually the usage of the shortname is shorter than using the whole issuer url. However, there are use
cases where this option might be quite useful. Generally it is a more universal way to obtain an access token for a
specific provider. While `oidc-token mySuperFancyShortname` might work on your machine it can fail for other users
because they do not have
`mySuperFancyShortname`. Using the issuer url

- when writing scripts that are shared with other users,
- opening issues that mention calls to `oidc-token`, or
- sharing other `oidc-token` related commands makes it easier for other users to run the same commands without any
  changes.

## Command Line Integration of oidc-token

If you have to pass an access token to another application on the command line you can substitute the token with ```
oidc-token <shortname>` ``. E.g. you can do an `curl` call with an OIDC token in the authorization header:

```
curl example.com -H 'Authorization: Bearer `oidc-token <shortname>`'
```

This syntax can be used with many applications and is quite useful.

## Obtaining More Information From oidc-token

As described under
[oidc-token](oidc-token/options.md#information-available-from-oidc-token) you can obtain more information when
calling `oidc-token` and not only the access token. If you want to do this we recommend the `--env` option and call
`oidc-token` the following way: ``eval `oidc-token --env <shortname>` ``. This way the environment variables `OIDC_AT`
, `OIDC_ISS`, and
`OIDC_EXP` or populated with the correct values.

A way that is **not** recommended, is to do multiple successive calls to `oidc-token` and only providing one of
the `--token`, `--issuer`, `--expires-at` options on each call. This would make three independent token requests
to `oidc-agent`. This is not only inefficient but also does not guarantee to return correct results. It might happen
that the token requested in the first call is only valid for a very short time and not valid anymore when doing the last
request; in this case a new token will be requested that has a different expiration time that does not relate to the
token from the first call.

## Autoloading and Autounloading Account Configurations

Since version `2.3.0` there is support for the so called `autoload` of account configurations. If an application
requests an access token for an configuration that is not loaded, it can be automatically loaded. The user will be
prompted to enter the encryption password for this account configuration and through that can decide if the account
should be loaded or not. This means we can do a call to `oidc-token example` and even if `example` is currently not
loaded, it will be loaded and an access token is returned.

The autoloading feature makes it quite easy to also use the autounload (limited lifetime of account configuration). When
starting `oidc-agent` with the
`--lifetime` option you can specify for how long account configuration should be loaded (the default is forever).
However, we now can use a limit and load account configuration only for e.g. 300 seconds. After that time the account
configuration will automatically be removed from the agent. But if an application needs an access token for an account
configuration it can be easy loaded through the autoload feature.

This way the time sensitive information is kept in memory by `oidc-agent`
can be limited without reducing usability much (the user does not always have to run `oidc-add`). Of course there are
use cases where the autounload-autoload combination is not useful, e.g. if a script runs periodically and needs an
access token and should run with no user interaction at all.

## Obtaining access tokens on a server

`oidc-agent` could be run on a remote server and then be used as usual to obtain access tokens. However, if you are
planning to do this, you should check your use case, if this is really necessary and if there might be a better
solution. Usually on of the following is a better solution::

- [Agent forwarding](configuration/forwarding.md) can be used to access a local agent on a remote server.
- The [mytoken service](https://mytoken-docs.data.kit.edu) is probably what you want.

When running oidc-agent on a server to obtain tokens, generating a new account configuration file on that server can be
more difficult, because there is neither a webbrowser nor a desktop environment. But because oidc-agent is designed for
command line usage, it is still possible. There are several ways of generating a new account configuration on a remote
server:

1. Generate it locally and copy it to the remote server
2. Using the password flow, which can be done entirely on the command line
3. Using the device flow, where a second device is used for the web-based authentication.
4. Using the authorization code flow with a 'manual redirect'

Options 2 and 3 are not supported by all providers. However, if the device flow is supported by your provider, we
recommend option 3.

When doing option 4 you should be aware of the following:

- Make sure that the agent uses the `--no-webserver` and `--no-scheme` options or pass these options to `oidc-gen`
- Also add the `--no-url-call` option when calling `oidc-gen`
- Copy the printed authorization url and open it in a browser (local).
- Authenticate and authorize oidc-agent as usual
- You will be redirected to localhost. Because there is no webserver listening your browser will display an error
  message.
- Copy the url to which you are redirected from the address bar of your browser
- Head over to the remote server and pass the copied url to `oidc-gen` in the following call:

```
oidc-gen --code-exchange='<url>'
```

## Updating an Expired Refresh Token

If a refresh token expired the user has to re-authenticate to obtain a new valid refresh token. Until version `2.3.0`
this would require the user to use
`oidc-gen -m <shortname>`, which allows the user to change all data of this account configuration (and therefore has to
confirm all existing data). Because the user only wants to re-authenticate to update the refresh token, confirming, that
all other data should be unchanged, is annoying.

Instead use
`oidc-gen --reauthenticate <shortname>`. This option will only start the re-authentication and update the refresh token.
Easier and faster.

## Applications that run under another user

On default only applications that run under the same user that also started the agent can obtain tokens from it. Whens
tarting the agent the `--with-group` option can be used to allow other applications to also access the agent. This can
be useful in cases where applications must run under a specific user.

The user first has to create a group (e.g. named `oidc-agent`) and add himself and all other users that need access to
the agent to this group. It is the user's responsibility to manage this group. Then he can pass the group name to
the `--with-group` option to allow all group members access to the agent. If the option is used without providing a
group name, the default is `oidc-agent`.

## Non-interactive oidc-gen

To run `oidc-gen` completely non-interactively (i.e. without user interaction)
one needs to pass several parameters:

- Pass all required information with command line arguments, e.g. `--iss`,
  `--scope`
- Use `--prompt=none` to disable prompting
- Use `--pw-file` or `--pw-cmd` to pass an encryption password
- Use `--confirm-default`, `--confirm-yes` or `--confirm-no` to automatically confirm with the default, yes or no.
