## General Usage
Account configurations are identified by their shortname, so an account
configuration can be added by using that shortname:
```
oidc-add <shortname>
```
The user will be prompted for the encryption password and then the account
configuration is loaded into the agent. After loading other applications can
request an access token for that account configuration from the agent.

```
Usage: oidc-add [OPTION...] ACCOUNT_SHORTNAME | -a | -l | -x | -X | -R
```

See [Detailed Information About All
Options](options.md) for more information.


