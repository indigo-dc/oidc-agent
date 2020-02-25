# Client Configuration Values
 When
registering a client manually you might have to provide quite a number of
specific configuration values. And even when using dynamic client registration
`oidc-gen` prompts you for some values. If you
are not familiar with one of these values, please check this section.

When registering a client an OpenID Provider might be using default values for
some of these configurations so you might not have to provide all of them.

## Scope
OpenID Connect clients use scope values to specify what access privileges are being requested for access tokens.
Required scopes for oidc-agent are: `openid` and `offline_access`. Additional scopes can be
registered if needed. Most likely you also want to register at least the
`profile` scope.

When using dynamic client registration the user will be prompted to enter scopes that
will be registered with that client. The keyword `max` can be used to
request all supported scopes.

Example Scope: `openid profile offline_access`

## Redirect Uri
The Redirect Uri is used during the Authorization Code Flow. The Redirect Uri must
be of the following scheme: `http://localhost:<port>` where `<port>` should be an
available port. It is also possible to specify an additional path, e.g.
`http://localhost:8080/redirect`, but this is not required. It is important that this port is not used when generating the
account configuration with oidc-gen. Multiple Redirect Uris can be registered to
have a backup port if the first one may be already in use. 
`oidc-gen` also supports a custom redirect scheme, that can be used to
redirect directly to oidc-gen. In that case the redirect uri has to be of the
form `edu.kit.data.oidc-agent:/<path>`.

We recommend registering the following redirect uris:
 - `http://localhost:4242`
 - `http://localhost:8080`
 - `http://localhost:43985`
 - `edu.kit.data.oidc-agent:/redirect`

Note: Only pass the `edu.kit.data.oidc-agent:/redirect` uri to oidc-gen, if
you wish to directly redirect to oidc-gen without using a webserver started by
oidc-agent.

## Response Type
The following response types must be registered:
- 'token' when using the Password Flow (see also
  [flow](oidc-gen.md#password-flow)) 
- 'code' when using the Authorization Code Flow (see also [flow](oidc-gen.md#authorization-code-flow))

## Grant Type
The following grant types must be registered:
- 'refresh_token' if available
- 'authorization_code' when using the Authorization Code Flow  (see also [flow](oidc-gen.md#authorization-code-flow))
- 'password' when using the Password Flow (see also
  [flow](oidc-gen.md#password-flow))
- 'urn:ietf:params:oauth:grant-type:device_code' when using the Device Flow (see also [flow](oidc-gen.md#device-flow))

