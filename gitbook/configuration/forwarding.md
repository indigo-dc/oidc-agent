## Agent Forwarding
When using `ssh` to connect to a remote server there might be the use case
where the user or an application on the remote server wants to receive an access token from
the local agent. This is possible by forwarding the UNIX domain socket used for
communicating with the agent. This can be done using the `-R` option of
`ssh`. Example:
```
ssh -R /tmp/oidc-forward:$OIDC_SOCK  user@host
```
However, if you do this you still have to manually export the `OIDC_SOCK`
environment variable on the server (`export
OIDC_SOCK=/tmp/oidc-forward`).

**To use agent forwarding we recommend the following configurations:**
Put the following in your `.bash_profile` on the server:
```
test -z $OIDC_SOCK && {
    export OIDC_SOCK=`/bin/ls -rt /tmp/oidc-forward-* 2>/dev/null | tail -n 1`
}
```
and this into your `.bash_logout` on the server:
```
if [ -e $OIDC_SOCK ]; then
    rm -f $OIDC_SOCK
fi
```

Add this to `.bash_aliases` on your local machine:
```
alias OA='echo -R /tmp/oidc-forward-$RANDOM:$OIDC_SOCK'
alias ssh-oidc="ssh \`OA\`"
```
You can then call ssh the following way:
```
ssh-oidc user@host
```
or
```
ssh user@host `OA`
```

