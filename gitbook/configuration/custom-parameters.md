# Custom Request Parameter

Since version `5.2.0` it is possible to customize the requests send by the agent to the OPs and add custom request
parameters.

Custom parameters can be configured in a config file named `custom_parameters.config`. As usual the file can be placed
in `/etc/oidc-agent` or the agent directory. If both are present parameters are merged together.

The `custom_parameters.config` contains a json array of parameter specifications. A parameter specification is a json
object that can have the following fields:

| Field Name    | Description                                                                                                                                                                                                                                                                                                                                                                                |
|---------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `parameter`   | The name of the parameter to be added to the request                                                                                                                                                                                                                                                                                                                                       |
| `value`       | The value that should be used. The value can be given in different ways. If the value starts with a `$` the following characters are interpreted as an environment variable and the value is read from this variable. If the given value starts with an `/` it is interpreted as a file path and the first line from that file is used as the value. Otherwise the value is used directly. |
| `for_issuer`  | A JSON array of issuer urls for which this parameter should be used                                                                                                                                                                                                                                                                                                                        |
| `for_account` | A JSON array of account shortnames for which this parameter should be used                                                                                                                                                                                                                                                                                                                 |
| `request`     | A JSON array of requests for which this parameter should be used. Possible values are `refresh`, `auth_url`, `code-exchange`,`device-init`,`device-polling`,`registration`,`revocation`,`password`                                                                                                                                                                                         |
