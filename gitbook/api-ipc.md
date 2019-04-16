## IPC-API
Alternatively an application can directly communicate with the oidc-agent through UNIX domain sockets. The socket address can be obtained from the environment variable which is set by the agent (```OIDC_SOCK```). The request has to be sent json encoded. We use a UNIX domain socket of type ```SOCK_STREAM```.

All Clients should ignore additional fields returned in a response from
oidc-agent, if the client does not understand these fields. Vice versa
oidc-agent ignores fields that it does not understand.

The following fields and values have to be present for the different calls:

### Access Token:
#### Request
| field            | value                                  | Requirement Level |
|------------------|----------------------------------------|-------------------|
| request          | access_token                           | REQUIRED          |
| account          | &lt;account_shortname&gt;              | REQUIRED if 'issuer' not used |
| issuer           | &lt;issuer_url&gt;                     | REQUIRED if 'account' not used |
| min_valid_period | &lt;min_valid_period&gt; [s]           | RECOMMENDED       |
| application_hint | &lt;application_name&gt;               | RECOMMENDED       |
| scope            | &lt;space delimited list of scopes&gt; | OPTIONAL          |

Note that one of the fields ```account``` and  ```issuer``` has to be present.
Use ```account``` to request an access token for a specific account
configuration and ```issuer``` when you do not know which account configuration
should be used but you do know the issuer for which you want to obtain an access
token. Do not provide both of these options in the same request.

Examples
The application ```example_application``` requests an access token for the account configuration ```iam```. The token should be
valid for at least 60 seconds and have the scopes ```openid profile phone```.
```
{"request":"access_token", "account":"iam", "min_valid_period":60,
"application_hint":"example_application", "scope":"openid profile phone"}
```

The application ```example_application``` requests an access token for the provider ```https://example.com/```. There are no guarantees that the token will be valid longer than 0 seconds and it will have all scopes that are avialable for the used account configuration.
```
{"request":"access_token", "issuer":"https://example.com/", "application_hint":"example_application"}
```

#### Response
| field        | value          |
|--------------|----------------|
| status       | success        |
| access_token | &lt;access_token&gt; |
| issuer       | &lt;issuer_url&gt; |
| expires_at       | &lt;expiration time&gt; |

example:
```
{"status":"success", "access_token":"token1234", "issuer":"https:example.com/",
"expires_at":1541517118}
```

#### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | &lt;error_description&gt; |

example:
```
{"status":"failure", "error":"Account not loaded"}
```

### List of Accounts:
The ability to retrieve the list of currently loaded accounts was removed with version 2.0.0.
