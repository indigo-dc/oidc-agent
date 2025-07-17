<!-- Template: -->
<!-- ### Features -->
<!--  -->
<!-- ### API -->
<!--  -->
<!-- ### Enhancements -->
<!--  -->
<!-- ### Bugfixes -->
<!--  -->
<!-- ### OpenID Provider -->
<!--  -->
<!-- ### Dependencies -->
<!--  -->

## oidc-agent 5.3.1

### Bugfixes

- Fixed an internal bug, that prevented building on macos

## oidc-agent 5.3.0

### Features

- The `issuer.config` file(s) now have support for a `user_client` object.
  This can be used to add a user registered client to an issuer and re-use
  accross account configurations.

### Enhancements

- Allow empty encryption password in GUI password prompts.
- In the refresh flow, `oidc-agent` now does not request the `offline_access` scope.
- When migrating from oidc-agent <5 the automatic update of the `issuer.config`
  file was improved. It can now correctly handle the case where an issuer
  existed with and without a trailing slash in the old file.

### Bugfixes

- Fixed a bug where `oidc-agent` would segfault if issuer.config files do
  not exist.

## oidc-agent 5.2.3

### Bugfixes

- Fixed a bug where `oidc-token` would segfault if the account was not known.

## oidc-agent 5.2.2

### Bugfixes

- Fixed a bug where `oidc-agent` would crash due to a segmentation fault if `~/.config/oidc-agent/issuer.config` was not
  present.

## oidc-agent 5.2.1

### Bugfixes

- Fixed permissions on static lib
- Removed bash completion for `oidc-tokensh`
- Fixed options passing from `oidc-agent-service` to `oidc-agent`

## oidc-agent 5.2.0

### Features

- Added possibility to add custom request parameters to requests done by the agent. This is done through
  a `custom_parameters.config` file placed in the agent dir or `/etc/oidc-agent`
- Added the capability to `oidc-agent` to restart after an update, i.e. when the oidc-agent binary changes, i.e. after a
  package update. This behavior is enabled through the `--restart-on-update` option.
- `oidc-agent-service` includes the `--restart-on-update` option on default in the `oidc-agent-service.options` file,
  i.e. auto-restart after update is enabled on default for agents started through `oidc-agent-service`. This can be
  disabled in the `oidc-agent-service.options` file.
- Added the `--bearer` and `--auth-header` options to `oidc-token`. These can be used to ease api calls.

## Changes

- Renamed the long option of `oidc-agent` `-a` from `--bind_address` to
  `--bind-address`.

### Change / Enhancement / Bugfix

The previous release stated that:

    When an account configuration is generated and the OP returns scopes in the initial token flow, the account
    configuration is updated with those scopes.

This did not work as intended. We made the following changes:

- Fixed a bug, so that the agent now actually behaves as described.
- Implemented separate scope lists for the initial token flow and the refreshing of tokens. Only the refresh-scope-list
  is updated. This way access tokens can be obtained with the correct (updated) scope, but re-authentication flows can
  still use the original scope list.

### Enhancements

- `oidc-add` can now also take an issuer url to load the default account for this issuer, i.e. `oidc-add <issuer_url>`
- `oidc-agent` now has a command line argument `--pid-file` to which the agent's pid is written.
- `oidc-agent-service` uses the new `--pid-file` option of `oidc-agent`
- If no socket path is set a default path is tried. The default path
  is `$TMPDIR/oidc-agent-service-$UID/oidc-agent.sock`, this is the path used by `oidc-agent-service`

### Bugfixes

- Fixed a bug where the ipc api would return always `success` when a mytoken is requested, even when this failed.
- Fixed some memory leaks

### OpenID Provider

- Added  https://alice-auth.cern.ch/
- Added https://atlas-auth.cern.ch/
- Added https://cms-auth.cern.ch/
- Added https://lhcb-auth.cern.ch/
- Added https://dteam-auth.cern.ch/

## oidc-agent 5.1.0

### Changes

- When an account configuration is generated and the OP returns scopes in the initial token flow, the account
  configuration is updated with those scopes.

### Features

- Added option to `oidc-add` to load an account config directly into the agent without the agent checking if it works.

### Bugfixes

- Fixed a problem with the tmp dir path that could occur on some windows systems that prevented oidc-agent to start.
- Fixed a potential segmentation fault when the oidc-agent dir is empty and account configs are listed.

## oidc-agent 5.0.1

### Bugfixes

- Fixed a missing define on MacOS that lead to a segfault when trying to start the agent

## oidc-agent 5.0.0

oidc-agent 5 is a major update that brings the power of a true configuration file and focuses on improving the user
experience and usability.
**See our [migration guide](https://indigo-dc.gitbook.io/oidc-agent/oidc-agent5) for details on how to
migrate to oidc-agent 5.**

### Changes

- Reworked the `issuer.config` file:
    - The `issuer.config` file in `/etc/oidc-agent` is updated on package upgrade
    - The `issuer.config` in user's oidc-agent dir is automatically updated when needed
    - The new format allows to set and tweak options / behavior on a per-issuer basis, e.g. if the encryption password
      should be stored.
- Dropped oidc-agent `--pw-lifetime` option. This did not work as expected. The intended usage can be achieved with
  the `issuer.config` file.
- Dropped support for storing encryption password in system's keyring (`--pw-keyring`)
    - This still can be done through `--pw-cmd`
- Changed the oidc-agent-service socket dir from `/tmp/oidc-agent-service/<uid>` to `/tmp/oidc-agent-service-<uid>`.
  This allows (better) multiple users to run oidc-agent-service.
    - This is a breaking change for all existing terminals that already have a `$OIDC_SOCK` set to a service socket. The
      easiest way to make sure that also existing sessions with the old path have access to a newly started agent,
      create a link from the old location to the new one, i.e.
  ```bash
  rm -rf /tmp/oidc-agent-service/${UID}/
  ln -s /tmp/oidc-agent-service-${UID} /tmp/oidc-agent-service/${UID}
  ```
- Also changed how the socket is managed by `oidc-agent-service`: Instead of linking the random socket location to a
  well known location, we now create the socket directly in the well known location. This improves security
  and `oidc-agent-service` can make use of the trust-checks on the socket location performed by the agent.

### Features

- Added support for RFC8707 to request ATs with specific audiences
    - Changed default audience request method to RFC8707
    - Old audience request behavior can be enabled for issuers through the `issuer.config` file.
    - For known IAM instances legacy aud mode is enabled by default
- Added support for `oidc-agent <command> [command_args]`, similar to ssh-agent; e.g. `oidc-agent bash` starts the agent
  and makes it available in a new bash.
- Added possibility for stat logging and sharing
    - Sharing usage statistics helps us better understanding how users use oidc-agent and therefore helps us to improve
      oidc-agent

### Security Fixes:

- Fixed permissions of agent socket.
- `oidc-agent` now checks the socket location to be trustworthy.

### API

- Added possibility to obtain (extended) account information from the agent. This includes all available accounts,
  associated to their OP issuer, an indicator if the account is loaded or not, and an indicator if there is a public
  client available for an issuer.
- Dropped deprecated functions from liboidc-agent
- Renamed numbered functions in liboidc-agent

### Enhancements

- A lot of the configuration options in the configuration file greatly improve the user experience, the following are
  just a few examples of what is possible:
    - Automatically store the encryption password for certain issuers
    - Automatically encrypt new account configuration with gpg
    - Automatically use a pre-registered client
    - Automatically prefer configurations via a mytoken server if issuer is available there
- Improved text and styling of prompts.
- Several improvements to the windows installer
- Improvements to the gui prompting design
- Several smaller improvements

### Bugfixes

- Fixed a bug that potentially could cause a segmentation fault
- Fixed a bug related to http retrying that potentially could cause a segmentation fault
- Fixed a problem in oidc-agent-service where only one user could run oidc-agent-service
- Fixed a bug where wrong unlock attempts of agent locking did not increase/create delay
- Fixed more bugs

### Dependencies

- Dropped libsecret dependency

### OpenID Provider

- Added OP: https://alice-auth.web.cern.ch/
- Added OP: https://atlas-auth.web.cern.ch/
- Added OP: https://cms-auth.web.cern.ch/
- Added OP: https://lhcb-auth.web.cern.ch/
- Added OP: https://bildungsproxy.aai.dfn.de
- Added public client for https://bildungsproxy.aai.dfn.de
- Added OP: https://auth.didmos.nfdi-aai.de
- Added public client for https://auth.didmos.nfdi-aai.de
- Added OP: https://regapp.nfdi-aai.de/oidc/realms/nfdi_demo
- Added public client for https://regapp.nfdi-aai.de/oidc/realms/nfdi_demo

## oidc-agent 4.5.2

### Changes

- Changed the `oidc-prompt` visuals because of CSS change

### Bugfixes

- Fixed a bug in `oidc-token` where the `-i` and `-e` options printed to `stderr` instead of `stdout` when a env var
  command was printed.
- Fixed missing urlencoding of request values in the auth code flow

### Dependencies

- `oidc-prompt` no longer uses `bootswatch` for css styling but `simplecss` instead.
- Updated `liblist`

### Other

- Improvements to the build process

## oidc-agent 4.5.1

### OIDC communication

- Added `client_id` parameter to code exchange request when a public client is used.
- Added `client_id` parameter to refresh flow request when a public client is used.

### Dependencies

- Added possibility to use mustache from shared lib

## oidc-agent 4.5.0

## Changes

- Support for local mytoken profiles is dropped. Instead, server-sided profiles are supported.

## oidc-agent 4.4.5

### OIDC communication

- Added `client_id` parameter to code exchange request when a public client is used.

### OpenID Provider

- Added OpenID Provider: https://proxy.eduteams.org
- Added public client for https://proxy.eduteams.org

## oidc-agent 4.4.4

### Bugfixes

- Fixed a bug where the `--issuer` option of `oidc-gen` was ignored when a mytoken server was used.

## oidc-agent 4.4.3

- Fixed a packaging bug on rpms
- Fixed a packaging / build bug on MacOS
- No changes for debs / windows

## oidc-agent 4.4.2

### Enhancements

- Added a retry for curl requests
- Reduced the (read) timeout to curl requests from 180 seconds to 10.
- Reduced the connect timeout of curl from 120 seconds to 5.

## oidc-agent 4.4.1

### Enhancements

- Added a (read) timeout to curl requests (180 seconds).
- Reduced the (default) connect timeout of curl from 300 seconds to 120.

### Bugfixes

- Fixed a bug where device code flow did not work correctly with public clients when OP requires client id in request
  body.

### OpenID Provider

- Removed decommissioned MITREid-based EGI OPs

## oidc-agent 4.4.0

### Features

- Added mytoken support:
    - Can use oidc-gen to create account configurations based on mytokens instead of refresh tokens. These account
      configuration can be used as usual to obtain ATs.
    - Additionally, mytoken based account configs can be used to obtain (sub-)mytokens.
    - Also includes support for mytoken profiles.

### Enhancements

- Added "oidc-agent <VERSION>" user agent header to all http requests done by the agent.
- Now can write more data to a pipe

### Bugfixes

- Fixed a bug where cli prompting for consent used the wrong default action (no instead of yes)
- Fixed the error message when trying to connect to a "non-existing" host
- Fixed a bug in liboidc-agent where error messages obtained through `oidc_errno` were not correct.
- Fixed a bug where the `openid` scope was always request in the oauth2 mode when using the `--only-at` option
  of `oidc-gen`, even tough it should not be used.

## oidc-agent 4.3.2

### Enhancements

- Improved the GUI check on MacOS, so that the device flow won't be used on default if GUI is available.
- Improved error message in case OP does not answer with json but json is expected.

### Bugfixes

- In `oidc-agent-service` use the correct path were oidc-agent is located after installation as default for the
  oidc-agent binary instead of `/usr/bin/oidc-agent`.
- Fixed a bug that lead to imprecise error message when something goes wrong during http
- Fixed a bug where the config files under `/etc/oidc-agent` could not be found in MacOS when they were placed into
  another directy as it is the case when installed via homebrew

## oidc-agent 4.3.1

### Bugfixes

- Fixed a bug where the oidc-prompt window displayed not as expected on tiling window managers.

## oidc-agent 4.3.0

oidc-agent 4.3.0 is a bigger release with some major changes and smaller fixes and enhancements.

### Windows

This is the first release with official support for Windows. We provide an installer that installs all needed tools and
libraries. While the windows version of oidc-agent works fine and can be used as a daily driver it is not as major as
the unix versions.

### oidc-prompt

The `oidc-prompt` tool was rewritten. The new tool now provides are more modern and consistent interface across
platforms. It also enables more advanced prompts which will be utilized in future versions.

### Seccomp

- Support for seccomp was **dropped** with this version.

### Other Features

- OAuth2 support:
    - `oidc-agent` does not only check `/.well-known/openid-configuration` but
      also `/.well-known/oauth-authorization-server` for server's metadata
    - For oauth2 account configurations `openid` is not a required scope
- Custom discovery/configuration endpoint
    - The `--config-endpoint` option of `oidc-gen` can be used to pass the uri of the server's configuration endpoint
    - This can be used for providers that do not advertise their metadata at one of the well-known location or not at
      all
    - A local file can be used by using an uri of the form `file:///path/to/file`
    - If a configuration endpoint is given the issuer url is no longer mandatory (since it can be read from the
      configuration endpoint)

### Enhancements

- Improved some build options, so oidc-agent should build with musl libc.
- Improved handling of the `--only-at` option.
- The `oidc-add` `-l` and `-a` option and the `oidc-gen` `-l` option now print the header line only if connected to a
  tty.
- `oidc-add` now checks if an account is already loaded before loading it (and prompting the user for a password).
  The `-f` option can be used to force a load even if the account is already loaded.
- `oidc-agent-service` now respects environment variables over values set in an `oidc-agent-service.options` file.
- `oidc-keychain` was rewritten to utilize `oidc-agent-service`
- Removed a superfluous error log message on the first account config generated.

### Bugfixes

- Fixed a bug where the `--only-at` option of `oidc-gen` was not working correctly and no AT was obtained if the OP did
  not send an RT, but only the AT
- Fixed a bug where an account configuration became unusable when the auto-reauthentication flow was triggered with the
  device flow, but not completed.
- Fixed a bug where `oidc-add -l` would print `Error: success` when the oidc-agent directory does not exist yet.

### OpenID Provider

- Issuer urls of some providers in the `issuer.config` were not correct (difference in a trailing slash) and have been
  fixed. This change only applies to the issuer url stored in `/etc/oidc-agent/issuer.config`. Issuer urls in
  the `issuer.config` file in the oidc-agent directory have to be updated by the user (this is optional, but
  recommended).
- Added the production, instance of the EGI-Checking keycloak based OP as issuers
- Added public client for production instance of the EGI-Checking keycloak based OP
- Replaced the demo and development instances of the EGI-Checking OP with the keycloak based one
- Replaced public clients for demo and development instances of the EGI-Checking with the keycloak based OP

### Dependencies

- `oidc-prompt` (oidc-agent-desktop packages) no longer depends on `yad` (`pashua` on MacOS), instead `gtk3`
  and `gtk-webkit2` are needed on linux)

## oidc-agent 4.2.6

### Bugfixes

- Fixed a bug where in the base64 decoding the wrong number was passed to the library function which on some platforms
  could lead to errors

## oidc-agent 4.2.5

### Other

- Fix formatting in gitbook

## oidc-agent 4.2.4

### Bugfixes:

- Fixed potential uncontrolled format string

## oidc-agent 4.2.3

### Bugfixes:

- Fixed cleanup of tmp directory for `oidc-agent-service`; in `4.2.2` we deleted too much

## oidc-agent 4.2.2

### Bugfixes:

- Fixed cleanup of tmp directory for `oidc-agent-service`
- Fixed typo that could cause a wrongly formatted error message

### Other

- Fixed a typo
- Fixed cast warning on libmicrohttpd >= 0.9.71

## oidc-agent 4.2.1

### Enhancements

- Encoding spaces printed authorization url, so it can be easily opened.

### Bugfixes

- Fixed problems on MacOS where automatic url opening did not work.

## oidc-agent 4.2.0

### Features

- Add option to encrypt account config file through gpg agent with an existing gpg key instead of using an encryption
  password
    - This feature comes very handy for accounts where the refresh tokens changes often (but can be used with any
      account configuration file)
    - To use gpg encryption when creating a new account include the `--gpg=<key_id>` option to your `oidc-gen` call
    - To update an existing account configuration to use gpg encryption run `oidc-gen -u <shortname> --gpg=<key_id>`
- Add Auto-re-authentication feature: When `oidc-agent` discovers that a refresh token expired it automatically triggers
  a re-authentication flow.

### API

- IPC-API:
    - The error response for an Access Token Request now might contain an `info` field. If present this field contains a
      formatted help message that gives instructions to the user how the problem can most likely be solved. Applications
      should display this message to the user if it is present.
- The `C` library `liboidcagent4` now has functions that return an `agent_response` that on error include the error and
  the help message. For details see https://indigo-dc.gitbook.io/oidc-agent/api/api-c#error-handling
- The `go` and `python` libraries have been adapted to support the help message. For details refer to:
    - https://indigo-dc.gitbook.io/oidc-agent/api/api-go
    - https://indigo-dc.gitbook.io/oidc-agent/api/api-py#error-handling

### Enhancements

- Now using `libqrencode` to print a QR code when using the device flow; instead of using `qrencode` only if already
  installed.
- Token revocation can now handle cases where there must be provided a `client_id` in the request.

### Bugfixes

- Fixed a bug where an error message was printed even tough no error occurred when `oidc-gen` tried to read a tmp file
  from `oidc-agent` and `oidc-gen` could not connect to agent.
- Fixed bug on MacOS where command line flags that are aliases would not accept argument
- Excluded `.log` files from account list
- Fixed bugs where some `--pw-*` options (mainly `--pw-file` and `--pw-env`) where not used by `oidc-agent`
- Fixed memory leaks in `oidc-agent`.
- Fixed handling of multiple OIDC flows by `oidc-agent`.
- Fixed bash completion on bullseye printing deprecation message
- Fixed potential TOCTOU filesystem race condition

<!-- ### OpenID Provider -->
<!--  -->

### Dependencies

- Now (directly) depending on `libqrencode` instead of optionally using `qrencode` binary.

## oidc-agent 4.1.1

### OpenID Provider

- Fixed scopes for EGI public clients
- Added compute.* scopes for WLCG public client
- Removed https://unity.eudat-aai.fz-juelich.de/oauth2/
- Added public client for B2ACCESS

## oidc-agent 4.1.0

### oidc-agent-server

- Support for `oidc-agent-server` has been dropped.

### Features

- Added option to `oidc-gen` to read the refresh token from environment variable.
- Added option to `oidc-gen` and `oidc-add` to read the encryption password from environment variable.
- Added option to `oidc-agent` to silence pid echo.
- Added option to `oidc-agent` to obtain env var values as json.
- Added option to `oidc-gen` to allow account generation without saving it.
- Added `oidc-agent-service` to easily start, stop, and restart an agent throughout a session.

### Enhancements

- Improved Xsession integration by using `oidc-agent-service`.
- Improved unexpected error message when account not loaded.
- Added success message at the end of `oidc-gen`.
- Public clients are now also read from the oidc-agent directory

### Bugfixes

- Fixed compilation issues on modern compilers
- Fixed `oidc-agent` output on `--status` if `$OIDC_SOCK` not set.

### Dependencies

- Update cJSON library.

## oidc-agent 4.0.2

### Bugfixes

- Fixed a json merge conflict when device authorization endpoint was set by user
- Fixed a bug where a message was printed to terminal when using the device flow when qrencode was not installed on the
  user's system

## oidc-agent 4.0.1

### Bugfixes

- Fixed a bug in liboidc-agent where getAccessTokenforIssuer never returned.
- Fixed agent forwarding with liboidc-agent.

## oidc-agent 4.0.0

### Incompatible Changes

- IPC encryption changed, therefore agents and clients (oidc-gen, oidc-add, oidc-token, etc.) must have the same major
  version to be able to communicate. **Agent must be restarted after updating!**
- Some options were removed from `oidc-gen`; these options are:
    - `--output` Splitting client configuration and agent account configuration is no longer supported.
    - `--qr` If `qrencode` is installed a QR code is automatically printed to the terminal.
    - `--qrt` If `qrencode` is installed a QR code is automatically printed to the terminal.
    - `--split-config` Splitting client configuration and agent account configuration is no longer supported.
    - `--clients` Splitting client configuration and agent account configuration is no longer supported.

### Features

- Add option `--only-at` to obtain AT through oidc-gen without creating an account configuration.
- Add oidc-agent-server an oidc-agent version that can run as a central server.
- `oidc-add` can now load locally existing configurations to a remote
  `oidc-agent-server`.
- `oidc-token` can also be used to obtain tokens from a remote
  `oidc-agent-server`.
- oidc-gen can now be used completely non-interactive
- Add `--pw-file` option to read decryption password from file
- Allow users to rename accounts.
- Add status command to oidc-agent to get information about the currently running agent.
- Add possibility to easily force a new AT through oidc-token.

### API

- Add encryption to liboidc-agent (now depends on libsodium).
- Also add encryption to the go and python library.
- The libraries now automatically support obtaining tokens from a remote
  `oidc-agent-server`.

### Enhancements

- User can now choose between cli and gui prompts (or none for `oidc-gen`).
- Add several new options for passing information to oidc-gen.
- When the 'max' keyword is used for scopes and a public client is used, this now uses the maximum scopes for that
  public client, not the issuer.
- Change how the symmetric key is derived in ipc communication to be able to support ipc encryption with golang lib.
- On default cnid (oidc-gen) is set to the hostname; so the hostname is included in the client name.
- Improve password prompt on autoload.
- Improve bash completion of oidc-gen short options.
- Delete oidc client when deleting agent configuration.
- Write temporary data to oidc-agent instead of tmp file.

### Bugfixes

- Fix a possible conflict between the application type 'web' and custom scheme redirect uris.
- Fix bug where oidc-gen would use a public client instead of aborting when generating an account configuration with a
  shortname that is already loaded.
- Fix duplicated output of oidc-agent when redirecting the stdout output.
- Fix segmentation fault in oidc-gen issuer selection when selecting 0
- Fix more segmentation faults.
- Fix memory leaks.

### OpenID Provider

- Add public client for aai-demo.egi.eu
- Add aai-demo.egi.eu

### Dependencies

- `liboidc-agent4` now depends on `libsodium`.
- Update cJSON library.

## oidc-agent 3.3.5

### OpenID Provider

- Add public client for login-dev.helmholtz.de/oauth2/

## oidc-agent 3.3.4

### OpenID Provider

- Add public client for dev.helmholtz.de/oauth2/

## oidc-agent 3.3.3

### Bugfixes

- Fix bash completion of shortnames if `$OIDC_CONFIG_DIR` is used.

### OpenID Provider

- Updated the issuer urls of HDF.

## oidc-agent 3.3.2

### Bugfixes

- Fix --pw-cmd not correctly working when output does not end with newline character
- Fix duplicated output of oidc-agent when redirecting
- Fix oidc-agent dies when client disconnects before agent can write back.

## oidc-agent 3.3.1

## Bugfixes

- Add a missing header line in the `oidc-add --loaded` output
- Remove dot files from configured account config listing.

## oidc-agent 3.3.0

### Features

- Add option to `oidc-add` to list currently loaded accounts.
- Add support to request tokens with specific audience.
- Add `--id-token` option to `oidc-token` to request an id-token from the agent.
- Add `oidc-keychain` to reuse `oidc-agent` across logins
- Add option to `oidc-token` to specify name of calling application.
- Add option to `oidc-agent` that allows log message printed to stderr.

### API

- Add the option to request access tokens with a specific audience to the `C`- `Go`- and `python`-libraries.

### Enhancements

- Add wlcg.cloud.cnaf.infn.it
- Add public client for wlcg.cloud.cnaf.infn.it
- Exit `oidc-gen` when error during scope lookup.
- Update cJSON library.

### Bugfixes

- Fix scope lookup not using cert path.
- Fix no-scheme option not working if first url is scheme url.
- Fix that some information is printed to stderr instead of stdout.
- Fix scopes not set when using password flow.
- Fix some minor bugs.

## oidc-agent 3.2.7

### Enhancements

- Improve RPM build

## oidc-agent 3.2.6

### Bugfixes

- Now adjusting X11settings only when the configuration file already exists.
- Fixed some spelling errors.

### Enhancements

- Increased `oidc-gen` polling interval and duration.
- `oidc-gen` now displays the scopes supported by the provider.
- Scopes provided to `oidc-gen` are no longer silently dropped when they are not advertised by the provider as
  supported.

## oidc-agent 3.2.5

### Bugfixes

- Fixed bug that might cause problems with providers that do not support PKCE. No longer sending code_verifier on auth
  code exchange requests.

## oidc-agent 3.2.4

### Enhancements

- Added new provider iam-demo.cloud.cnaf.infn.it/
- Added public client for iam-demo.cloud.cnaf.infn.it

## oidc-agent 3.2.3

### Enhancements

- Added public client for deep datacloud
- Added public client for extreme datacloud

## oidc-agent 3.2.2

### Enhancements

- Add possibility to avoid custom uri scheme (useful when running on a remote server)
- Now displaying warning message when client registration could not register all requested scopes.

### Bugfixes:

- Fixed bug with doubled communication when not all required scopes could be registered

## oidc-agent 3.2.0

### Features

- Added the possibility to allow applications that run under another user to obtain tokens from the agent, when starting
  the agent with the [`--with-group`](https://indigo-dc.gitbooks.io/oidc-agent/content/oidc-agent.html#-with-group)
  option

## oidc-agent 3.1.2

### Bugfixes

- Fixed a bug due to which no error message was displayed when trying to load an account configuration and the
  oidc-agent directory was not accessible for oidc-add.
- This bug also caused the agent to crash if oidc-token was used to load this account configuration on the fly and the
  oidc-agent directory was not accessible for oidc-agent.

## oidc-agent 3.1.1

### Bugfixes

- Fixed a bug that did not save the information from dynamic client registration (did not save merged data).

### Dependencies

- Updated the cJSON library

## oidc-agent 3.1.0

- Support on MacOS

## oidc-agent 3.0.2

### Bugfixes

- Fixed behavior of oidc-gen -p when the passed file does not exist.
- Fixed segfault if the issuer.config in the oidc-agent directory doesn't exist and an AT is requested by issuer.
- Fixed a segfault if the pubclients.conf file does not exist

## oidc-agent 3.0.1

### Provider

- Added the elixir public client to the list of public clients

## oidc-agent 3.0.0

### Features

- Support for [agent forwarding](https://indigo-dc.gitbooks.io/oidc-agent/configure.html#agent-forwarding)
- Support for default account configuration for providers:
    - Defaults can be set in the `issuer.config` file in the oidc-agent directory
    - Other applications can request access tokens by the issuer (IPC-API, liboidc-agent)
    - `oidc-token` can be used with issuer url

### API

- **Incompatible!** Changed the type of the oidc-agent socket from `SOCK_SEQPACKET` to `SOCK_STREAM`
- Added `getAccessToken2` to liboidc-agent; should be used if only an access token is requested
- Added `getAccessTokenForIssuer` and `getTokenResponseForIssuer` to liboidc-agent to request access tokens by issuer
  and not by shortname.

## oidc-agent 2.3.1

### Bugfixes

- Fixed the course of a bug that would not utilize the cached AT when an application requests an AT with an empty scope
  value. This fix might have also fixed other unknown bugs.
- Improved the user prompt message for autoload when the application does not send an application_hint
- Fix a bug related to the confirm feature: after a request is declined it was impossible to get an access token for
  this configuration without reloading the configuration.

### Enhancements

- Improved error handling when a wrong refresh token is used

## oidc-agent 2.3.0

### Features

- Autoload: If an application requests an access token for an account configuration that is not yet loaded the user can
  be prompted to load it and then the application can receive the requested access token. No need to run `oidc-add`
  preventively. See also
  the [Tips section in the documentation](https://indigo-dc.gitbooks.io/oidc-agent/tips.html#autoloading-and-autounloading-account-configurations)
  .
- Confirmation: When loading an account configuration with `oidc-add` the new `-c`/`--confirm` option can be used.
  Similar to `ssh-add` this option requires confirmation by the user whenever the account configuration should be used,
  i.e. whenever an application requests an access token for that account configuration the user will be prompted if he
  wants to allow or deny this usage. The option can also be turned on for all configuration loaded into the agent when
  specifying this option on agent startup.
- Changing refresh token: A provider might decide that it issues a new refresh token whenever an access token is issued.
  In that case `oidc-agent` has to update the account configuration file. To do this the agent requires the encryption
  password. The agent supports user prompting, keeping it encrypted in memory, reading it from a user provided command,
  and saving it in the system's keyring.
- Custom uri schemes: By using a redirect uri of the form `edu.kit.data.oidc-agent:/<path>` the agent can skip the
  normally started httpserver and redirect directly to `oidc-gen` to complete the account configuration generation
  process.
- Manual redirect: The auth code flow can now be done completly without the httpserver started by `oidc-agent`. Either
  through usage of a custom uri scheme redirect url or by manually copying the url the user is redirect to from the
  browser and passing it to `oidc-gen --codeExchange='<url>'`.
- XSession integration: `oidc-agent` is now integrated with Xsession to automatically be available in all terminals
  throughout an Xsession.

### Changes

- Changed the underlying architecture by splitting `oidc-agent` internally into two components
- Changed the `oidc-agent` flag for console mode from `-c` to `-d`
- Changed the default port for redirect urls registered with dynamically registered clients from `2912` to `4242`

### Enhancements

- When the auth code flow fails at the redirect because of problems with the httpserver, the url can be passed manually
  to `oidc-gen --codeExchange='<url>'`
- When a refresh token expired the user has to reauthenticate to obtain a new valid refresh token. Instead of
  using `oidc-gen -m` to do this the user can also use the new `oidc-gen --reauthenticate` option (the user won't have
  to confirm that all other data should not be changed).
- The `oidc-gen -u` option that updates an encrypted file to the newest encryption and file format version can now also
  be used with unencrypted files
- When using `oidc-gen -d` the account config now does not have to be loaded. The refresh token can also be revoked if
  not loaded.
- Improved the [documentation](https://indigo-dc.gitbooks.io/oidc-agent/)
- Communication between the agent and its httpserver is now encrypted
- Improved usability of `oidc-gen` with some smaller enhancements at various places
- Other smaller enhancements

### OpenID Provider

- Added a public client for HBP
- Added a public client for Elixir

### Bugfixes

- Fixed some memory leaks
- Fixed a segmentation fault that would happen when an agent with a public client loaded is locked
- Fixed other theoretically possible segmentation faults
- Other smaller fixes

## oidc-agent 2.2.6

### Bugfixes

- Removed an unnecessary client_id from post data, that caused problems with iam when using the device flow.

## oidc-agent 2.2.5

### Bugfixes

- Fixed a bug that made it impossible to use the device flow

## oidc-agent 2.2.4

### Bugfixes

- Fixed a possible seg fault
- Fixed a bug with file location that use the oidcdir specified in the `OIDC_CONFIG_DIR` env var, if that value does not
  have a trailing slash

## oidc-agent 2.2.3

### Bugfixes

- Fixed a bug that might have leaked sensitive information to the system log (see #176)
- Added the `profile` scope back to default scopes during oidc-gen

### Enhancements

- Added an option to manually specify the redirect port used during dynamic client registration (`--port`)
- Made the location of the oidcagentdir customizable using the `OIDC_CONFIG_DIR` environment variable

## oidc-agent 2.2.2

### Provider

- Added public client for aai.egi.eu

## oidc-agent 2.2.1

### Bugfixes

- Improved error message when necessary scopes cannot be registered during dynamic client registration
- If necessary scopes cannot be registered during dynamic client registration, a public client is tried
- Fixed memory leaks
- Allow updating of public clients by using the -m and --pub option

## oidc-agent 2.2.0

### Features

- Support for PKCE
- Public clients: If dynamic client registration is not supported by a provider, public clients can be used (for some
  providers) so that a user does not have to register its own client manually.

### Bugfixes

- Fixed some code flaws
- Fixed seg fault when dynamic client registration failed
- Fixed more possible seg faults
- Improved error handling when authorization flow not possible
- Fixed a bug where it was possible to display issuer urls that only differ in the trayling slash twice when using
  oidc-gen
- Enforce usage of openid and offline_access scope in all cases
- Fixed a bug due to which oidc-agent would return a wrong already loaded account config when generating a new account
  config

### Packages

- Support for RPM packages

## oidc-agent 2.1.3

### Bugfixes

- Fixed superfluous error logs when checking if a string is a json object
- Fixed strange additional parameters in the auth code exchange request
- Fixed a problem with unity OP where access token did not have any scope
- Fixed build error if bin dir not existed

### Enhancements

- Changed encoding for memory encryption from hex to base64

## oidc-agent 2.1.2

### Bugfixes

- Fixed a bug due to which errors during token revocation were ignored
- Fixed a bug displaying a (wrong) error message when token revocation succeeded and the server answered with an empty
  response. This bug was introduced with encrypted ipc communication.
- Fixed a bug where the browser would not redirect to the werbser when the chosen port was to high -> Now explicitly
  checking the port range when the user provides the redirect url
- Fixed a segmentation fault if the config tmp file did not contain the account shortname
- Fixed bash completion that would fail if oidcdir does not exist (yet)

## oidc-agent 2.1.1

### Bugfixes

- Fixed a bug causing problems with the device flow
- Fixed memory leaks

## oidc-agent 2.1.0

### Features

- Added possibility to update a configuration file to the newest file format / encryption: `oidc-ggen -u <FILE>`
- Encrypted IPC: oidc-gen and oidc-add now encrypt all communication with oidc-agent

### Enhancements

- Now using base64 encoding instead of hex encoding for all new encryptions
- Updated the file format for configuration file. Storing all important encryption parameters and also the version with
  which it was generated.
- When building from source the libcjson package can be used over the local files using `make HAS_CJSON=1`
- Using `oidc-gen --dae` now enforces registration of the needed grant type, even if the provider does not advertise it
  as supported.
- Improved the account listing output.

### Library & API

- We now also provide a shared library (see also [Packaging](#packaging-dependencies))

### Bugfixes

- Fixed some segmentation faults that were possible
- Fixed oidc-agent responding twixe when a check request was sending while being locked
- Fixed some memory leaks
- Fixed some possibilities for double frees
- Fixed missing authorization for device access token requests
- Fixed invalid read in stringToJSON when parsing fails
- Fixed a wrongly included grant_type parameter in the authorization code url.
- Fixed incompatibilities between account configuration files that were generated with oidc-agent using different
  versions of libsodium.

### Packaging & Dependencies

- Removed the user dependency for libsodium. Now linked as a static library
- We now provide addition packages: `liboidc-agent2` and `liboidc-agent-dev` for the oidc-agent library

## oidc-agent 2.0.3

### Behavior Changes

- seccomp is now disabled on default. It can be enabled with the `--seccomp` option. The `--no-seccomp` option was
  removed.

### Bugfixes

- Fixed a bug that autoremoved also accounts with infinite lifetime when an account with limited lifetime expired.
- Added missing seccomp syscalls
- Fixed a bug that broke bash completion
- Fixed possible segmentation faults

### Other Changes

- increased the maximum length of error message
- Disabled Tracing: Cannnot longer attach using ptrace

## oidc-agent 2.0.2

### Bugfixes

- Fixed a bug that disabled seccomp for oidc-add and oidc-token
- Fixed a bug where modifying the default scope (dyn client reg) could fail the client registration.

### Enhancements

- Internal Improvements to bash-completion

## oidc-agent 2.0.1

### Bugfixes

- Fixed a bug related to merging json objects
- Fixed a missing seccomp syscall

### Enhancements

- Improved oidc-gen user interface:
    - oidc-gen now does not prompt for a refresh token on default. Instead the `--rt` option can be used.
    - oidc-gen now only prompts for credentials if the password flow is used (`--flow=password`)
- Improved internal flow handling of dynamic client registration

## oidc-agent 2.0.0

### Features

- **Combined Configuration File:**
  When using dynamic client registration the default behavior is now to generate only one configuration file containing
  both client configuration and account configuration.

  **Under very rare conditions this might break an old configuration file.**
  If this happens, use `oidc-gen -p <shortname>` to display the decrypted content. You can then use this information to
  generate a new account configuration (using `oidc-gen -m`).

- **Account Lifetime:**
  Added to possibility to set a lifetime for account configurations. After this time the account is automatically
  removed from the agent. It is possible to set a default lifetime for all account configurations when
  starting `oidc-agent` using the new `-t` option. It is also possible to specify a lifetime with `-t` when loading a
  configuration with `oidc-add`.

- **Better Support for Turning Colors Off:**
  It is now possible to turn colors off in different ways:
    - set the `NO_COLOR` environment variable: Color support is turned off if this variable is presented (regardless of
      its value).
    - set `TERM` to `dumb`: color support is turned off if the `TERM` variable is set to `dumb`.
    - set `OIDC_AGENT_NOCOLOR` to a non zero value.

  Colors can be turned on for oidc-agent regardless of the above mentioned variables by setting the `OIDC_AGENT_NOCOLOR`
  environemnt variable to `0`. Furthermore color is turned off if not connected to a tty (e.g. if output redirected to a
  file).

- **Memory Encryption:**
  Sensitive Information is obfuscated in memory to improve security.

- **Agent Lockable:**
  Added the possibility to lock the agent. When locked the agent refuses any operation until it is unlocked. While being
  locked additional encryption is applied to the sensitive information kept in memory.

- **Seccomp:**
  Restricted the set of syscalls that can be made by each component. If this feature causes problems on a specific
  system it can be turned off with the `--no-seccomp` option.

- **List Currently Loaded Account Configurations:**
  This feature was removed.

- **Automatically Open Authorization URL:**
  Added possibility to turn off the automatic opening of the authorization url (authorization code flow) using
  the `--no-url-call` option.

- **Unloading Accounts:**
  Unloading an account configuration does not require the password anymore. Also added an option to unload all loaded
  account configuration at once.

- **oidc-token:**
  Added the possibility to not only get an access token with `oidc-token` but also get the associated issuer and the
  expiration time of this token. To do so the new `-o`, `-i`, `-e`, `-a`, and `-c` options can be used. This also allows
  calling oidc-token with `eval` to directly set one or multiple environment variables.

### Changes to the CLI

- Added support for bash completion
- No longer using space delimited lists. To provide multiple values for an option the option can be provided multiple
  times.

### API

#### C-API

- Removed `char* getLoadedAccounts()`: It is not possible anymore to get the list of currently loaded configuration from
  the agent.
- A TokenResponse now includes the token, the issuer, and the expiration date.
- A TokenRequest should include an application hint. For detailed information refer to
  the [documentation](https://indigo-dc.gitbooks.io/oidc-agent/api.html)

#### IPC-API

- Removed the `account_list` request. Applications that use this request to check if an account is loaded before
  requesting an access token for it, should simply request the access token. If the account is not loaded, an error is
  returned.
- Access token request should now include an `application_hint`.
- The Response to a token request now includes the expiration time of the token (as well as the token and the associated
  issuer url). For detailed information refer to the [documentation](https://indigo-dc.gitbooks.io/oidc-agent/api.html)

### Bugfixes

- Fixed a bug where conflicting response types were registered.
- Fixed a bug where the automatic account configuration generation failed after dynamic client registration.
- Fixed a bug where only the first 4096 bytes of an ipc message were sent.
- Fixed a bug related to token revocation.
- Fixed a bug with empty IPC messages.
- Fixed numerous bugs added during development.
- Fixed some smaller bugs.

### Dependencies

- The json parser was changed to cJSON
- Dependencies are not longer included as static library but included in this repo

## oidc-agent 1.3.0

### Bugfixes

- Fixes static library

### Enhancements

- Hides client secret
- Validation for redirect url format
- Optionally prints the device code url QR-code directly to the terminal
- Adds optional client name identifier when using dynamic registration

### API

- Backward-compatible API-change: ipc access token requests now also contain the associated issuer; also the C-API
  includes it

## oidc-agent 1.2.7

### Bugfixes

- fixed segmentation fault for an unchecked file existence

### Provider

- Added DEEP
- Added HDF

## oidc-agent 1.2.6

### Library

- Now providing C-API as a static library
- oidc-token uses that library

### Provider

- Added KIT

## oidc-agent 1.2.1

- Support for providing the device authorization endpoint manually

## oidc-agent 1.2.0

### Features

- Support for Authorization Code Flow
- Support for Device Flow
- Support to choose used flow
- Support for user defined scopes
- List account configurations
- List client configurations
- Print decrypted file content
- Colored output

### API

#### C-API

- The function `getAccessToken` has an additional parameter scope. It can be used to pass a space delimited list of
  scope values. To use the default scope values pass NULL.

#### IPC-API

- When performing a token request the field min_valid_period is now optional instead of required. The default value is
    0.
- When performing a token request the new optional field scope can be used to provide a space delimited list of scope
  values.

### Enhancements

- yes

### Bugfixes

- yes

## oidc-agent 1.1.0

### Features

- Dynamic registration (`oidc-gen -r`) is now the default option for oidc-gen. If a user does not want to use dynamic
  client registration `oidc-gen -m` can be used.

### API

- Provider configurations are renamed to account configurations. This effects the API in fields like `account_list`

### Bugfixes

- fixes agent's response when it could not get a refresh token. It was success; changed now to failure.

## oidc-agent 1.0.5

### Features

- Adds the `-c` flag for oidc-agent. It will skip the daemonizing.

### IPC-API

- The provider list is now returned as JSON Array of Strings.
- Changed the socket type from SOCK_STREAM to SOCK_SEQPACKET

## oidc-agent 1.0.4

### Bugfixes

- Fixed bug where oidc-agent would crash if it receives non-json message

## oidc-agent 1.0.3

### Bugfixes

- Fixed segfault
- Fixed bug where the client config file was not saved
- Fixed that the encrypted client config file could not be used by oidc-gen -f

## oidc-agent 1.0.0

First release of oidc-agent, including oidc-gen, oidc-add, oidc-token and a client api.
