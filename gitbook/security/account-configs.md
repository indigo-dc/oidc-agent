## Account Configuration Files

The generated account configuration files contain sensitive information (i.e.
client credentials and the refresh token) and are therefore stored in an
encrypted way.

There are two options to encrypt account configurations:

- via `gpg-agent`
- password-based

All encryption (except `gpg`-based) done in the `oidc-agent` project is done through the
[`libsodium library`](https://github.com/jedisct1/libsodium), which is
also used by software such as `Discord`, `RavenDB`, or `Wire`.

The encryption uses an `XSalsa20` stream cipher.


