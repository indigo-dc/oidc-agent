## Helmholtz Data Federation (HDF)
HDF does not support dynamic client registration and you have to register a
client manually: 
- Make sure you don’t have an active login in unity and visit the /home endpoint (i.e. https://login.helmholtz-data-federation.de/home )
- Click “Register a new account” on the top right
- Specify the required information and note that “User name” is your `client_id` and “Password credential” is your `client_secret`.

Note also that you have to enter at least one valid redirect uri, even if they
are not mandated by HDF (see [Client Configuration Values](#redirect-uri) for
more information).

After the client is registered, call oidc-gen with the `-m` flag and enter the
required information. 


