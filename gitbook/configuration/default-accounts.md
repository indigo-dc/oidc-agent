## Default Account Configuration for a Provider
The `issuer.config` file in the [oidc-agent directory](directory.md) can also
be used to set an default account configuration file for each provider by adding
the shortname of this account configuration after the issuer url.
A line in the `issuer.config` file should look the following:
```
<issuer_url>[<space><shortname>]
```

