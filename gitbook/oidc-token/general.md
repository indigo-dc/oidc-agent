## General Usage
To obtain an access token for a specific account configuration you have to pass
the shortname to oidc-token:
`oidc-token <shortname>`
This will print the access token to `stdout`.
This enables serveral use cases:
- print the token to `stdout` and copy paste it where you need it.
- put the token in an environment variable: ``export OIDC_AT=`oidc-token shortname` ``.
- pipe the token to a programm that reads a token from `stdin`: `oidc-token shortname | iReadATokenFromStdIn`.
- use the `oidc-token` directly in the needed command: ``curl -H 'Authorization: Bearer `oidc-token shortname`' example.com``.
- use the `-c` (or similar) option to put the token into an environment
  variable: ``eval `oidc-token -c <shortname>` ``

Instead of using `oidc-token <shortname>` you also can do `oidc-token
<issuer_url>`. While usually using the shortname is shorter there are also use
cases for using the issuer url.

See also [Tips](../tips.md) for more usage tips.

```
Usage: oidc-token [OPTION...] ACCOUNT_SHORTNAME | ISSUER_URL
```

See [Detailed Information About All
Options](options.md) for more information.


