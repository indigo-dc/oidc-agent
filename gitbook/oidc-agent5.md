## Migrating to oidc-agent 5

oidc-agent 5 is a major release with quite some usability improvements but unfortunately also some breaking changes.
However, most things should just work as you are used to.

**All users of `oidc-agent-service` (this includes the default integration into the system on Linux) need to make a
little adjustment to be able to access the new agent from existing terminal sessions.**
The socket path location of `oidc-agent-service` was changed, therefore the old location has be linked to the new one.
**After** restarting the agent with `eval $(oidc-agent-service restart)`, run the following commands:

```bash
rm -rf /tmp/oidc-agent-service/${UID}/
ln -s /tmp/oidc-agent-service-${UID} /tmp/oidc-agent-service/${UID}
```

While we recommend to take a few minutes to configure version 5 of oidc-agent, so it suits your needs best, further
adaptions are not required.
After restarting the agent you could just use version 5 of oidc-agent as you are used to without any modifications.

Nevertheless, it is a good idea to go through the following points:

### Adapt Configuration

Copy the new global configuration file `/etc/oidc-agent/config` to your user's oidc-agent
directory (`~/.config/oidc-agent` or `~/.oidc-agent`).
Go through the file and adapt it to your needs - but of course you can also keep the default configuration;
the default configuration represents what you are used to from oidc-agent 4 (you can also delete entries that you do not
change, but you don't have to).

One option we would like to draw your attention to is the `default_gpg_key` option under `oidc-gen`. If you set a gpg
key id there, all newly created account configurations will be encrypted with that gpg key. No encryption passwords are
needed.

#### Statistics

Version 5 of oidc-agent can collect information about the requests it receives and build up a request log.
If you share this data with us, we can better understand how oidc-agent is used by our users and improve it further.
All information collected is completely anonymized and **no security sensitive data is collected**.
You can check yourself what information is collected by looking into the $OIDCDIR/oidc-agent.stats file.
In our opinion, the most sensitive piece of information is the `country` where you use oidc-agent. Collection of this
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
  "machine_id": "0000000fbf182739b3849d8200000000_1000",
  "boot_id": "00000000-b84b-4f7d-b8f2-600000000000_1000",
  "os_info": "Linux 6.1.0-10-amd64 #1 SMP PREEMPT_DYNAMIC Debian 6.1.38-2 (2023-07-27) x86_64 GNU/Linux",
  "time": 1693301191,
  "lt_offset": 7200,
  "agent_version": "5.0.0",
  "country": "DE",
  "request": "access_token",
  "account": "kit",
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