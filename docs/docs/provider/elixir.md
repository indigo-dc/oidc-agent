## Elixir
Elixir supports dynamic registration, but dynamically registered clients
will not have any scopes. Therefore users have to either register a client
manually (and get approval for the needed scopes) or use a preregistered public client (recommended).

Example:
```
$ oidc-gen --pub <shortname>
[...]
Issuer [https://login.elixir-czech.org/oidc/]:
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

### Advanced options

#### Manual Client Registration
If you register a client manually, please see https://docs.google.com/document/d/1vOyW4dLVozy7oQvINYxHheVaLvwNsvvghbiKTLg7RbY/

#### Device Flow
To use the device flow with Elixir, the client has to have the device grant type
registered. This is the case for our public client, however, it might most
likely not be the case for a manually registered client.
To use the device flow instead of the authorization code flow with the
preregistered public client include the `--flow=device --pub` options.

