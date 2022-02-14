## Agent Forwarding
When using `ssh` to connect to a remote server, you may also need to receive access tokens from
the local agent. This is possible by forwarding the UNIX domain socket used for
communicating with the agent.

This needs a **client** and **server** side configuration. Both
configurations can be done by a local user.

### Client
This can be done using the `-R` option of
`ssh` (e.g. with `ssh -R /tmp/oidc-forward:$OIDC_SOCK  user@host`).
Defining an alias makes this more easily usable:
. Example:
```
alias ssh-oidc='ssh -R /tmp/oidc-forward-$RANDOM:$OIDC_SOCK'
ssh user@host
```
For most convenience, you should put the alias line into your `.profile`, `.zshrc`,
or `.bash_aliases` file.

**Note** that you could also overwrite the actual `ssh` command with an
alias. While this works, this will always create a socket file on the
remote host, which can be used by the remote system administrator to
access your tokens. Use it wisely


### Server
On the server, you have to set the `OIDC_SOCK`
environment variable (`export OIDC_SOCK=/tmp/oidc-forward`).

**We recommend the following configurations:**
Put the following in your `.profile`, `.zshrc`, or `.bash_profile` on the server:
```
test -z $OIDC_SOCK && {
    export OIDC_SOCK=`/bin/ls -rt /tmp/oidc-forward-* 2>/dev/null | tail -n 1`
}
alias ssh-oidc='ssh -R /tmp/oidc-forward-$RANDOM:$OIDC_SOCK'
```

Add this into your `.zlogout`, or `.bash_logout` on the server:
```
if [ -e $OIDC_SOCK ]; then
    rm -f $OIDC_SOCK
fi
```

