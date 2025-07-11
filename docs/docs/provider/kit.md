## KIT
The KIT OIDP supports dynamic client registration, but a special access token is
required as authorization. The easiest way is too use the preregistered public
client.

### Quickstart
Example:
```
$ oidc-gen --pub <shortname>
[...]
Issuer [https://oidc.scc.kit.edu/auth/realms/kit/]:
Space delimited list of scopes [openid profile offline_access]:
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

The KIT OpenID Provider issues a new refresh token when the current refresh
token was used in the refresh flow (whenever a new access token is issued). When the refresh token
  changes oidc-agent has to update the client configuration file and therefore
  needs the encryption password. Because with rotating refresh tokens, this will
  happen quite often it is recommended to allow oidc-agent to keep the password
  in memory by specifying the `--pw-store` option when loading the account
  configuration with `oidc-add`.

### Advanced options
To get an initial access token please contact the [provider](https://www.scc.kit.edu/dienste/openid-connect.php).
The token can then be used as authorization through the `--at` option.


