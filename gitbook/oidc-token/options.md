## Detailed information about all options

* [`--time`](#seccomp)
* [Information Available from oidc-token](#information-available-from-oidc-token)
  * [`--all`](#seccomp)
  * [`--env`](#seccomp)
  * [`--expires-at`](#expires-at)
  * [`--issuer`](#issuer)
  * [`--token`](#token)
* [`--aud`](#aud)
* [`--id-token`](#id-token)
* [`--name`](#name)
* [`--scope`](#scope)
* [`--seccomp`](#seccomp)

### `--time`
Using the `--time` option you can specify the minimum time (given in seconds) the access token
should be valid. If this options is not given, it will be zero, therefore no guarantees about the validity of the token can be made, i.e. the access token might not be valid anymore even when
used immediately.
The agent will check if the cached token is still valid for the specified time
and return it if that is the case. Otherwise a new
access token is issued and returned.
oidc-agent guarantees that the token will be valid
the specified time, if it is below the provider's maximum, otherwise it will be the
provider's maximum (i.e. if `--time=3600` is used, but for that provider
access tokens are only valid for 5 minutes, the returned token will be valid for
those 5 minutes).

### Information Available from oidc-token
On default `oidc-token` prints the requested access token to `stdout`.
But `oidc-token` can provide more information, like the issuer url of the
issuer for which the access token is valid. This information might be required
by other applications, so that they know where the token has to be used.
Additionally the time when the token expires (as the number of seconds since the Epoch, `1970-01-01 00:00:00 +0000 (UTC)`) can also be returned. This enables
an application to cache the token for the time it is valid.

There are multiple ways to obtain all of this information or only a subset using `oidc-token`:
In the following we will describe different command line options that can be
used to control the returned information. For additional examples refer to
[Tips](tips.md).


- Use the `-a` option to get all information: oidc-token will print all
  information to `stdout`. One piece of information per line:
- Use environment variables: Using the `-c` option oidc-token will print out
  shell commands that can be evaluated to set environment variables (name of the
  environment variables are defaults):

  ``eval `oidc-token <short_name> -c` `` will automatically set these
  variables. Using the `-o`, `-i`, and `-e` option the name of the
  exported variables can be customized.


#### `--all`
To get all information available and print it to `stdout` use the
`--all` option. Each line contains one piece of information:
  - First line: access token
  - Second line: issuer url
  - Third line: expiration time

This way it is easy to parse on the command line or by other applications.
However, on the command line you might prefer the usage of environment variables
(`--env`).

#### `--env`
Instead of printing all information directly to `stdout` the `--env`
option prints out shell commands that will put all information into environment
variables.
Therefore, it can be used to easily make all information available in the current
terminal: ``eval `oidc-token -c <shortname>` ``

The names of the used environment variables are as followed:
  - `OIDC_AT`: access token
  - `OIDC_ISS`: issuer url
  - `OIDC_EXP`: expiration time

The name of the environment variables can be changed with the
`--expires-at`, `--issuer`, and `--token` options.

#### `--expires-at`
The `--expires-at` option can be used to request the time when the access
token expires (given in the number of seconds since the Epoch, `1970-01-01
00:00:00 +0000 (UTC)`). It optionally takes the name of an environment variable
as an argument. If this argument is not passed and non of the `--issuer` and
`--token` options are passed, the expiration time is printed to
`stdout`. Otherwise shell commands are printed that will export the value
into an environment variable. The name of this variable can be set with the
passed argument and defaults to `OIDC_EXP`.

Examples:
```
oidc-token <shortname> -e               # prints the expiration time
eval `oidc-token <shortname> -oe`       # puts the access token and expiration time into OIDC_AT and OIDC_EXP, resp.
eval `oidc-token <shortname> -e AT_EXP` # puts the expiration time into AT_EXP
```

#### `--issuer`
The `--issuer` option can be used to request the issuer url of the issuer that issued the access token.
It optionally takes the name of an environment variable
as an argument. If this argument is not passed and non of the `--expires-at` and
`--token` options are passed, the issuer url is printed to
`stdout`. Otherwise shell commands are printed that will export the value
into an environment variable. The name of this variable can be set with the
passed argument and defaults to `OIDC_ISS`.

Examples:
```
oidc-token <shortname> -i               # prints the issuer url
eval `oidc-token <shortname> -oi`       # puts the access token and issuer url into OIDC_AT and OIDC_ISS, resp.
eval `oidc-token <shortname> -i ISSUER` # puts the issuer url into ISSUER
```

#### `--token`
The `--token` option can be used to request the access token.
It optionally takes the name of an environment variable
as an argument. If this argument is not passed and non of the `--expires-at` and
`--token` options are passed, the access token is printed to
`stdout` (same as when no options are provided). Otherwise shell commands are printed that will export the value
into an environment variable. The name of this variable can be set with the
passed argument and defaults to `OIDC_AT`.

Examples:
```
eval `oidc-token <shortname> -oi`      # puts the access token and issuer url into OIDC_AT and OIDC_ISS, resp.
eval `oidc-token <shortname> -o TOKEN` # puts the issuer url into TOKEN
```

### `--aud`
The `--aud` option can be used to request an access token with the specified
audience. Protected resources should not accept a token if they are not listed
as audience. Therefore, this is a mechanism to restrict the usage of an access
token to certain resources.

Note that the format of providing multiple audiences might be different for
different providers, since this parameter is currently not widely supported by
providers and a clear standard is not yet established. We currently only know
about one provider that supports this parameter (IAM); there multiple audiences
can be requested as a space separated string.

Example:
```
oidc-token <shortname> --aud="foo bar"
```

### `--id-token`
The `--id-token` option can be used to request an id token instead of an
access token. Note that id tokens should not be passed to other applications
as authorization. This option is only mend as a toll for development, it
should not be used by other applications.
Therefore, this option requires user approval, unless the account
configuration was
loaded with `oidc-add --always-allow-idtoken` or the
`--always-allow-idtoken` option was specific on agent startup.

### `--scope`
The `--scope` option can be used to specify the scopes of the requested token. The returned
access token will only be valid for these scope values. The flag only takes one scope value, but multiple values can be passed by using this option multiple times. All passed scope values have to be registered for this client; upscoping is therefore not possible.

Example:
```
oidc-token <shortname> -s openid -s profile
```

If this option is omitted the default scope is used.

### `--name`
The `--name` option is intended for other applications and scripts that call `oidc-token` to obtain an access token. The option sets the passed name as the application name that requests the access token. This name might be displayed to the user, e.g. when the account first has to be loaded. Setting the correct application name allows the user to decide on correct information.

Example:
```
oidc-token <shortname> --name="My custom script"
```

### `--seccomp`
Enables seccomp system call filtering. See [general seccomp
notes](security.md#seccomp) for more details.

