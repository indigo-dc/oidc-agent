## Google
Google does not support dynamic client registration, but there is a
preregistered public client so that account configuration generation is as easy
as with dynamic client registration.

### Quickstart
Example:
```
$ oidc-gen --pub --issuer https://accounts.google.com/ <shortname> 
[...]
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

#### Manual Client registration
A client can be registered manually at <https://console.developers.google.com/> There is documentation
on how to do this at
<https://developers.google.com/identity/protocols/OpenIDConnect> (just the first
section "Setting up OAuth 2.0").

After the client registration you can download the client configuration as a
json file. You can pass this file to `oidc-gen` using the `-f` flag. If you
don't do this you have to enter the configuration manually (you than have to
call `oidc-gen` with the `-m` flag).

#### Device Flow
When using the Device Flow with Google you have to call oidc-gen with the
`--flow=device` option. Additionally for Google you have to provide the
device authorization endpoint using the `--dae` option. Note also that the
registered OIDC client has to be of type `native`/`other` and not `web
application`.

Example call for using the device flow with Google:
```
oidc-gen google -m --flow=device --dae=https://accounts.google.com/o/oauth2/device/code
oidc-gen google --pub --flow=device --dae=https://accounts.google.com/o/oauth2/device/code
```


