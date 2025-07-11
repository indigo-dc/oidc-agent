## IAM (INDIGO/DEEP/WLCG)
IAM supports dynamic registration and a simple call to oidc-gen is therefore
enough to register a client and generate the account configuration.

### Quickstart
Example:
```
$ oidc-gen <shortname>
[...]
Issuer [https://iam-test.indigo-datacloud.eu/]:
Space delimited list of scopes [openid profile offline_access]:
Registering Client ...
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

### Advanced options
Instead of using the authorization code flow one could also
use the password flow or device flow instead.

#### Password Flow
Using IAM the password grant type is not supported in dynamic client registration. The client is registered without it
and you have to contact the provider to update the client config manually. After that is
done, you can run oidc-gen again with the same shortname. oidc-gen should find a temp file and continue the account configuration generation. Afterwards the config is added to oidc-agent 
and can be used by oidc-add normally to add and remove the account configuration from the agent.
You have to provide the `--flow=password` option to all calls to `oidc-gen`.

#### Device Flow
To use the device flow with IAM simply include the `--flow=device` option when
calling `oidc-gen`.

