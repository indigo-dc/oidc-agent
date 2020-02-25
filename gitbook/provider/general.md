## A Provider not Listed
If your provider was not listed above, do not worry - oidc-agent should work with any OpenID Provider. Please
follow these steps.

### Try Dynamic Client Registration
If you already have a registered client you can see [Generate the
Account Configuration](#generate-the-account-configuration).

Dynamic client registration is not supported by all OpenID Providers, so it
might fail. Anyway, try registering a client dynamically by calling oidc-gen and
providing the issuer url when being prompted.

If dynamic client registration is not supported, oidc-gen will tell you this.
In that case you have to register the client manually through the provider's web
interface (see [Client Configuration Values](#client-configuration-values) for help with manual client registration) and then go to [Generate the Account Configuration](#generate-the-account-configuration).

Some providers have a protected registration endpoint which is not public. If so
oidc-agent needs an initial access token for authorization at the endpoint.
Please call oidc-gen with the `--at` option to pass the access token to
oidc-gen.

When the client was successfully registered the account configuration should be
generated automatically and you should be able to save and use it.

### Generate the Account Configuration
If you registered a client manually call oidc-gen with the `-m` flag or if
you have a file containing the json formatted client configuration pass it to
oidc-gen with the `-f` flag.

After entering the required information oidc-agent should be able to generate
the account configuration which is then usable.

### Still no Success?
If you still were not be able to get oidc-agent working with that provider,
please contact the provider or us at <https://github.com/indigo-dc/oidc-agent/issues>. We will
try to figure out if the problem is with oidc-agent or the provider.

