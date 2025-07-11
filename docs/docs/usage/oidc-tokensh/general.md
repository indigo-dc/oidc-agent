## General Usage

`oidc-tokensh` is a tool to ensure that valid Access Tokens are always
available in a location such as `$XDG_RUNTIME_DIR/bt_u$ID`,
`/tmp/bt_u$ID`, or `$BEARER_TOKEN_FILE` just as specified
<https://zenodo.org/records/3937438>.

`oidc-tokensh` provides an "almost drop-in replacement" for `httokensh` of
the [htgettoken](https://github.com/fermitools/htgettoken) tool package.

`oidc-tokensh` starts a new shell through `oidc-agent` and prompts the user
for the passphrase of the `oidc-agent shortname` that will be loaded.

The user may specify the `shortname` with the `--oidc <shortname>` option.
If only one `shortname` is configured, this one will be used by default.

```
Usage: oidc-tokensh [--oidc <shortname>] [-- <command>]
```


See [Detailed Information About All
Options](options.md) for more information.
