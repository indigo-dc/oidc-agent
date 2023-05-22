## Migrating to oidc-agent 5

While we recommend to take a few minutes to configure version 5 of oidc-agent, so it suits your needs best, it is not
required.
After restarting the agent you could just use version 5 of oidc-agent as you are used to without any modifications.

Nevertheless, it is a good idea to go through the following points:

### Adapt Configuration

Copy the new global configuration file `/etc/oidc-agent/config` to your user's oidc-agent
directory (`~/.config/oidc-agent` or `~/.oidc-agent`).
Go through the file and adapt it to your needs - but of course you can also keep the default configuration;
the default configuration represents what you are used to from oidc-agent 4.

One option we would like to draw your attention to is the `default_gpg_key` option under `oidc-gen`. If you set a gpg
key id
there, all newly created account configurations will be encrypted with that gpg key. No encryption passwords are needed.

#### Statistics

Version 5 of oidc-agent can collect information about the requests it receives and build up a request log.
If you share this data with us, we can better understand how oidc-agent is used by our users and improve it further.
All information collected is completely anonymized and **no security sensitive data is collected**.
You can check yourself what information is collected by looking into the $OIDCDIR/oidc-agent.stats file.
In our opinion, the most sensitive piece of information is the `country` where oidc-agent is used. Collection of this
claim can be disabled/enabled separately.

**By default, no information is collected.**
Requests logs are only collected by the agent if the `stats_collect_local` is set to `true` in the config file.
Furthermore, these are only shared with us if additionally the `stats_collect_share` option is set to `true`.
Collection of the request country can be controlled with the `stats_collect_location` option.

Since it really helps us to get a better understanding on how oidc-agent is used in practice we kindly ask you to enable
the collection and sharing of these information.

An example request entry looks the following:

```json
{
  "machine_id": "00000000-93ca-11d2-aa0d-000000000000_1000",
  "boot_id": "00000000-4552-461d-ae24-000000000000_1000",
  "os_info": "Linux 5.10.0-23-amd64 #1 SMP Debian 5.10.179-1 (2023-05-12) x86_64 GNU/Linux",
  "time": 1684737794,
  "lt_offset": 7200,
  "agent_version": "5.0.0",
  "country": "DE",
  "request": "access_token",
  "account": "wlcg",
  "application_hint": "oidc-token",
  "min_valid_period": 0
}
```

### Restart the agent

It's now time to restart your agent with the new version 5.

```bash
eval `oidc-agent-service restart`
```

### Load your accounts

While this is not required, we recommend you to load all your accounts once after migrating to version 5. This way the
agent can build the new `issuer.config` file in your oidc-agent directory and map all your account configs to the
correct issuer. This is useful when clients request access tokens for an issuer rather than for a specific account
config.

### Update encryption

If you configured a default gpg key in the config and want to migrate existing password encrypted account configs to
gpg-based encryption you can do so by running

```bash
oidc-gen -u <shortname>
```

for each of the relevant account configs.

### Special Issuer Needs?

If you want to configure different behavior for some issuers have a look at
the [`issuer.config` file](configuration/issuers.md).
If you make changes, please restart the agent again.