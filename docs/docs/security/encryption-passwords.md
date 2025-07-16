## Encryption Passwords

Generally, the encryption password provided by the user to encrypt / decrypt
account configurations is kept in memory as short as possible. However, `oidc-agent` can keep them longer in memory (
encrypted) so it can update the configuration file when a provider issues a new refresh token. This option is not
enabled on default and has to be enabled explicitly by the user.
With oidc-agent 5 this option can be set on a per-issuer basis, and is enabled by default for certain issuers.

If the user did not enable any password caching feature and `oidc-agent`
needs an encryption password because it has to update the account
configuration file, the user will be prompted for the password. However,
if a provider uses rotating refresh tokens, this might be impractical,
because the user has to enter his encryption password whenever a new
access token is issued. We therefore implemented different password
caching features:

- `oidc-agent` can keep the encryption password in memory. The encryption
  password will be well encrypted; and the password used for this
  encryption is dynamically generated when the agent starts. To use this approach use the `--pw-store` option of
  `oidc-add` or `oidc-agent` or (recommended) adapt the `issuer.config`.
- `oidc-agent` can also retrieve the encryption password from a user
  provided command; the output of this command will be used as the
  encryption password. The command used will be kept encrypted in
  memory, because it is used to obtain the encryption password (potentially) without
  any additional checks, it should be treated the same way as the
  encryption password itself. Because with this option the user (potentially) does not
  have to enter his password at any point (also not when loading the
  account configuration with `oidc-add`) it might be especially useful
  when writing scripts. To use this approach use the `--pw-cmd` option
  of `oidc-add` or `oidc-gen`.
- `oidc-agent` can also retrieve the encryption password from a user
  provided file; the content of this file will be used as the encryption
  password. The filepath used will be kept encrypted in memory, because
  it is used to obtain the encryption password without any additional
  checks, it should be treated the same way as the encryption password
  itself. Because with this option the user does not have to enter his
  password at any point (also not when loading the account configuration
  with `oidc-add`) it might be especially useful when writing scripts.
  To use this approach use the `--pw-file` option of `oidc-add` or
  `oidc-gen`.

