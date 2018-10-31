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
| field            | value                            | Requirement Level |
|------------------|----------------------------------|-------------------|
| request          | access_token                     | REQUIRED          |
| account          | \<account_shortname\>              | REQUIRED          |
| min_valid_period | \<min_valid_period\> [s]           | RECOMMENDED       |
| scope            | \<space delimited list of scopes\> | OPTIONAL          |

example:
```
{"request":"access_token", "account":"iam", "min_valid_period":60,
"scope":"openid profile phone"}
```

##### Response
| field        | value          |
|--------------|----------------|
| status       | success        |
| access_token | \<access_token\> |
| issuer       | \<issuer_url\> |

example:
```
{"status":"success", "access_token":"token1234", "issuer":"https:example.com/"}
```

##### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | \<error_description\> |

example:
```
{"status":"failure", "error":"Account not loaded"}
```

#### List of Accounts:
The ability to retrieve the list of currently loaded accounts was removed with version 2.0.0.


