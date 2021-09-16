## Google

Google does not support dynamic client registration, but there is a preregistered public client so that account
configuration generation is as easy as with dynamic client registration.

### Quickstart

Example:

```
$ oidc-gen --pub --issuer https://accounts.google.com/ --flow=device <shortname> 
The following scopes are supported: openid profile
Scopes or 'max' (space separated) [openid profile]: 
[...]
Using a browser on any device, visit:
https://www.google.com/device
[...]
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.

Enter encryption password for account configuration '<shortname>': 
Confirm encryption Password: 
```

### Advanced options

#### Manual Client registration

A client can be registered manually at <https://console.developers.google.com/> There is documentation on how to do this
at
<https://developers.google.com/identity/protocols/OpenIDConnect> (just the first section "Setting up OAuth 2.0").

After the client registration you can download the client configuration as a json file. You can pass this file
to `oidc-gen` using the `-f` flag. If you don't do this you have to enter the configuration manually (you than have to
call `oidc-gen` with the `-m` flag).



