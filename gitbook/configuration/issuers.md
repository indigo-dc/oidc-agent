# Configuration of Providers

Prior to oidc-agent version `5.0.0` the `issuer.config` file was used to have a list of issuers (OpenID Providers)
that `oidc-gen` used as suggestions.
It could also be used to set a default account for each issuer. A separate file `pubclients.config` was used to
configure public clients.

In oidc-agent 5 and beyond these files have been merged into a single, more powerful configuration file about issuers.
The `issuer.config` file can contain a json array of json objects each describing an issuer. It is also possible to
split configuration of issuers into separate files.
The `issuer.config.d` directory can contain config files that each hold the json object configuration for one issuer.

oidc-agent combines the issuer configuration from these locations (the lowest entry has the highest priority):

- `/etc/oidc-agent/issuer.config.d/*`
- `/etc/oidc-agent/issuer.config`
- `$OIDC_CONFIG_DIR/issuer.config.d/*`
- `$OIDC_CONFIG_DIR/issuer.config`

An issuer config object can have the following fields:

| Field Name                      | Description                                                                                                                                                            |
|---------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `issuer`                        | The issuer url                                                                                                                                                         |
| `manual_register`               | A url where a client can be registered manually                                                                                                                        |
| `contact`                       | Contact information for this issuer                                                                                                                                    |
| `configuration_endpoint`        | The url of the configuration endpoint if it cannot be derived from the issuer url                                                                                      |
| `device_authorization_endpoint` | The url of the device authorization endpoint if it is not published at the configuration endpoint                                                                      |
| `cert_path`                     | The local certificate bundle path that should be used when communicating with the issuer                                                                               |
| `store_pw`                      | Indicates if the encryption password should be kept in memory, so that the account configuration file can be updated without prompting the user for the password again |
| `oauth`                         | Indicates that this is an oauth2 instead of an OIDC issuer                                                                                                             |
| `pubclient`                     | Information about a public client for this issuer                                                                                                                      |

Additionally, the following properties are supported, but should only be given in the `issuer.config` file located in
the oidc-agent directory.

- `default_account`: The name of the default account config; if not given the first account config in the `accounts`
  field is used as a default.
- `accounts`: A list of all the available accounts for this issuer; MUST not be edited manually, this field is managed
  by
  the agent.

The `pubclient` field is an object that can have the following fields:

| Field Name      | Description                                                                                                           |
|-----------------|-----------------------------------------------------------------------------------------------------------------------|
| `client_id`     | The client id of the public client                                                                                    |
| `client_secret` | If given the client secret of the public client                                                                       |
| `scope`         | The scopes available for this public client                                                                           |
| `flows`         | The oidc flows supported by this public client; possible values are the same as for the `--flow` option of `oidc-gen` |
