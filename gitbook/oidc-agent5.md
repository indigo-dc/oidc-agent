## Migrating to oidc-agent 5

While you should take a few minutes to configure version 5 of oidc-agent and ensure that everything works the way that
is most suited for you, it is not required.
After restarting the agent you could just use version 5 of oidc-agent without any modifications.

Nevertheless, it is a good idea to go through the following points:

### Adapt Configuration

Copy the new global configuration file `/etc/oidc-agent/config` to your user's oidc-agent
directory (`~/.config/oidc-agent` or `~/.oidc-agent`).
Go through the file and adapt it to your needs - but of course you can also leave the default configuration; this is
what you are used to from oidc-agent 4.

One option we would like your attention to is the `default_gpg_key` option under `oidc-gen`. If you set a gpg key id
there, all newly created account configurations will be encrypted with that gpg key. No encryption passwords are needed.

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