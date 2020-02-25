# oidc-gen
`oidc-gen` is used to generate new account configuration. These account
configurations are needed and used by oidc-agent. They can be loaded with
`oidc-add` into the agent. And then any application can request an access
token for that account configuration.

Account configurations are identified by a shortname. This shortname can be set
to anything, but it is recommended to use a descriptive name of the provider /
account used. E.g. a shortname for an account configuration for the DEEP Hybrid
Datacloud could be 'deep'; for Google it could be 'google' or if a user has
multiple Google accounts it could be something like 'google-work' and
'google-personal'.
Usually it is enough the generate such an account configuration only once.

For `oidc-gen` there are a lot of options. We will cover all of them in
detail under the point [Detailed Information About All
Options](#detailed-information-about-all-options). To get help with generating
an account configuration for a specific provider refer to [How to get an
account configuration with
...](provider.md#how-to-get-an-account-configuration-with) or if you have to
register a client manually refer to
[Client Configuration Values](provider.md#client-configuration-values).

