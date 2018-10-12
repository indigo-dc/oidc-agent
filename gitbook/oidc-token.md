# oidc-token
oidc-token is an example agent client using the provided C-API and can be used to 
easily get an OIDC access token from the command line. oidc-token can also list the
currently loaded accounts.

```
$ oidc-token --help
Usage: oidc-token [OPTION...] ACCOUNT_SHORTNAME | -l
oidc-token -- A client for oidc-agent for getting OIDC access tokens.

 General:
  -l, --listaccounts         Lists the currently loaded accounts
  -t, --time=SECONDS         Minimum number of seconds the access token should
                             be valid

 Advanced:
  -s, --scope=SCOPE          Space delimited list of scopes to be requested for
                             the requested access token

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <https://github.com/indigo-dc/oidc-agent/issues>.
```

To get an access token for an account you have to specify the short name and
how long the access token should be valid at least. The time is given in
seconds. If no minimum period of validity is specified, the default value 0 will
be used. This means that the access token might not be valid anymore even when
be used instantly. If the current access token is not valid long enough, a new 
access token is issued and returned. We guarantee that the token will be valid 
the specific time, if it is below the provider's maximum, otherwise it will be the 
provider's maximum.

The following call will get an access token for the account with the short name
'iam'. The access token will be valid at least for 60 seconds.
```
oidc-token iam -t 60
```

To save the access token in an environment variable you can use the following
command:
```
export ACCESS_TOKEN=`oidc-token <short_name>`
```
## oidc-token and Scopes
The ```--scope``` flag can be used to specify specific scopes. The returned
access token will be only valid for these scope values. The flag takes a space
delimited list of scope values that has to be a subset of the scope values
registered for this client.

If the flag is not provided the default scope is used.

# Other agent clients
Any application that needs an access token can use our API to get an access token from 
oidc-agent. The following applications are already able to get an access token from oidc-agent:
- [wattson](https://github.com/indigo-dc/wattson)
- [orchent](https://github.com/indigo-dc/orchent)
