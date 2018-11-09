## API
### C-API
The C-API provides functions for getting an access token for a specific configuration as well as the
associated issuer. These functions are designed for easy usage. The C-API is available as a static library at [GitHub](https://github.com/indigo-dc/oidc-agent/releases).

### IPC-API
Alternatively an application can directly communicate with the oidc-agent through UNIX domain sockets. The socket address can be obtained from the environment variable which is set by the agent (```OIDC_SOCK```). The request has to be sent json encoded. We use a UNIX domain socket of type ```SOCK_SEQPACKET```.

All Clients should ignore additional fields returned in a response from
oidc-agent, if the client does not understand these fields.

The following fields and values have to be present for the different calls:

#### Access Token:
##### Request
| field            | value                              | Requirement Level |
|------------------|------------------------------------|-------------------|
| request          | access_token                       | REQUIRED          |
| account          | &lt;account_shortname&gt;              | REQUIRED          |
| min_valid_period | &lt;min_valid_period&gt; [s]           | RECOMMENDED       |
| application_hint | &lt;application_name&gt;            | RECOMMENDED       |
| scope            | &lt;space delimited list of scopes&gt; | OPTIONAL          |

example:
```
{"request":"access_token", "account":"iam", "min_valid_period":60,
"application_hint":"example_application", "scope":"openid profile phone"}
```

##### Response
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

##### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | &lt;error_description&gt; |

example:
```
{"status":"failure", "error":"Account not loaded"}
```

#### List of Accounts:
The ability to retrieve the list of currently loaded accounts was removed with version 2.0.0.


