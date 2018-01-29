## API
The C-API provides functions for getting a list of currently loaded account 
configurations and an access token for a specific configuration. They can be 
easily used. 

Alternatively an application can directly communicate with the oidc-agent through UNIX domain sockets. The socket address can be get from the environment variable which is set by the agent (```OIDC_SOCK```). The request has to be sent json encoded. We use a UNIX domain socket of type ```SOCK_SEQPACKET```.
The following fields and values have to be present for the different calls:

### List of Accounts:
#### Request
| field   | value         | Requirement Level |
|---------|---------------|-------------------|
| request | account_list  | REQUIRED          |

example:
```
{"request":"account_list"}
```

#### Response
| field         | value                 |
|---------------|-----------------------|
| status        | success               |
| account_list  | JSON Array of strings |

example:
```
{"status":"success", "account_list":["iam", "test"]}
```

#### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | <error_description> |

example:
```
{"status":"failure", "error":"Bad Request: could not parse json"}
```

### Access Token:
#### Request
| field            | value                            | Requirement Level |
|------------------|----------------------------------|-------------------|
| request          | access_token                     | REQUIRED          |
| account          | <account_shortname>              | REQUIRED          |
| min_valid_period | <min_valid_period> [s]           | RECOMMENDED       |
| scope            | <space delimited list of scopes> | OPTIONAL          |

example:
```
{"request":"access_token", "account":"iam", "min_valid_period":60,
"scope":"openid profile phone"}
```

#### Response
| field        | value          |
|--------------|----------------|
| status       | success        |
| access_token | <access_token> |

example:
```
{"status":"success", "access_token":"token1234"}
```

#### Error Response
| field  | value               |
|--------|---------------------|
| status | failure             |
| error  | <error_description> |

example:
```
{"status":"failure", "error":"Account not loaded"}
```
