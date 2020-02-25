## Human Brain Project (HBP)
HBP supports dynamic registration, but has a protected registration endpoint. 
Therefore, a user has to be a member of the Human Brain Project and has to pass an initial access token to oidc-gen using the `--at` option. One way to obtain such an access token is using [WaTTS](https://watts.data.kit.edu/).

Example:
```
$ oidc-gen <shortname> --at=<access_token>
[...]
Issuer [https://services.humanbrainproject.eu/oidc/]:
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

Alternatively it is also possible to use a preregistered public client by using
the `--pub` option (`--at` is not required in that case).


