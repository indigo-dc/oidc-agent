## Helmholtz AAI
Helmholtz AAI does not support dynamic client registration, but there is a
preregistered public client which can be used so that account configuration is as easy
as with dynamic client registration.

### Use Preregistered Public Client

Enter the following command and follow the instructions to take advantage of the preregistered public client:
```
$ oidc-gen --pub --issuer https://login.helmholtz.de/oauth2/ \
    --scope "email \
             eduperson_scoped_affiliation \
             eduperson_unique_id \
             eduperson_assurance \
             eduperson_entitlement" \
    <shortname>
```
You will need to follow the OIDC-flow, which usually involves
authentication in a web-browser. If the browser does not start, you can
copy paste the displayed URL. 

```
$ oidc-gen --pub --issuer https://login.helmholtz.de/oauth2/ <shortname>
[...]
Space delimited list of scopes [openid profile offline_access]:
Generating account configuration ...
accepted
To continue and approve the registered client visit the following URL in a Browser of your choice:
https://[...]
[...]
Polling oidc-agent to get the generated account configuration .....
success
The generated account config was successfully added to oidc-agent. You don't have to run oidc-add.
```

Finally, you will be be asked for a password on
the commandline to safely store your credentials.

```
Enter encryption password for account configuration '<shortname>':
Confirm encryption Password:
```

**Note**: You need to run the webbrowser on the same host as the
`oidc-gen` command.
\
If you operate on a remote machine, you need to use the
device code flow, by adding `--flow=device` to the above commandline.

Advanced users may succeed by otherwise ensuring that the browser you are using can connect to the host on
which `oidc-gen` and `oidc-agent` run on ports 4242, 8080 or 43985.


### Manual Client registration

- Make sure you **don’t** have an active login in unity and visit the /home endpoint (i.e. https://login.helmholtz.de/home )
- **Don't login**
- Click "Register a new account" on the top right
- Click "Oauth2/OIDC client Registration"
- Specify the required information, but note the following:
    - "User name" is your `client_id`
      - Needs to be globally unique, "`oidc-agent`" will clash. Use
          "`oidc-agent_<your name>`" instead. You Must Not include a colon.
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
are not mandated by Helmholtz AAI (see [Client Configuration
Values](client-configuration-values.md#redirect-uri) for
more information).

After the client is registered, call oidc-gen with the `-m` flag and enter the
required information.
