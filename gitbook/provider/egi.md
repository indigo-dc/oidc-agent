## EGI Check-in
EGI Check-in supports dynamic registration, but dynamically registered clients
will not have any scopes. Therefore users have to either register a client
manually or use a preregistered public client (recommended).

Example:
```
$ oidc-gen --pub oidc-gen --pub --issuer https://aai.egi.eu/oidc --scope "eduperson_entitlement email" <shortname>
[...]
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
Polling oidc-agent to get the generated account configuration .....success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.

Enter encryption password for account configuration '<shortname>':
Confirm encryption Password:
```

Note that you need to run the webbrowser on the same host as the
`oidc-gen` command. 

Advanced users may succeed by otherwise ensuring that the browser you are
using can connect to the host on which `oidc-gen` and `oidc-agent` run on
ports 4242, 8080 or 43985.

<!--This only happens once a year with EGI-->
<!--### Advanced options-->
<!--If you register a client manually you have the option to disable 'Refresh tokens-->
<!--for this client are reused'. If you disable this option each refresh token can-->
<!--  only be used once. Therefore, a new refresh token will be issued after each-->
<!--  refresh flow (whenever a new access token is issued). When the refresh token-->
<!--  changes oidc-agent has to update the client configuration file and therefore-->
<!--  needs the encryption password. Because with rotating refresh tokens, this will-->
<!--  happen quite often it is recommended to allow oidc-agent to keep the password-->
<!--  in memory by specifying the `--pw-store` option when loading the account-->
<!--  configuration with `oidc-add`. -->
