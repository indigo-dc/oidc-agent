## Helmholtz Data Federation (HDF)
HDF does not support dynamic client registration and you have to register a
client manually: 
- Make sure you **don’t** have an active login in unity and visit the /home endpoint (i.e. https://login.helmholtz.de/home )
- **Don't login**
- Click "Register a new account" on the top right
- Specify the required information, but note the following:
    - "User name" is your `client_id`
      - Needs to be globally unique, "`oidc-agent`" might clash. Use
          "`oidc-agent:<your name>`" instead.
    - "Password credential" is your `client_secret`
    - "Service Admin Contact": `Your email address`
    - "Service Security Contact": `Your email address`
    - "Service DPS URL": `https://github.com/indigo-dc/oidc-agent/blob/master/PRIVACY`
    - "Service Description": `https://github.com/indigo-dc/oidc-agent`
    - "OAuth client return URL (1)": `http://localhost:4242`
        - click "`+`" to add more URLs
    - "OAuth client return URL (2)": `http://localhost:8080`
    - "OAuth client return URL (3)": `http://localhost:43985`
    - "OAuth client return URL (4)": `edu.kit.data.oidc-agent:/redirect`

Note also that you have to enter at least one valid redirect uri, even if they
are not mandated by HDF (see [Client Configuration
Values](client-configuration-values.md#redirect-uri) for
more information).

After the client is registered, call oidc-gen with the `-m` flag and enter the
required information. 


