# oidc-token-exchange
oidc-token-exchange is <!-- TODO -->

```
$ oidc-token-exchange --help
Usage: oidc-token [OPTION...] ACCOUNT_SHORTNAME
oidc-token -- A client for oidc-agent for getting OIDC access tokens.

 General:
  -t, --time=SECONDS         Minimum number of seconds the access token should
                             be valid

 Advanced:
  -a, --all                  Return all available information (token, issuer,
                             expiration time). Each value is printed in one
                             line.
  -c, --env                  This will get all available information (same as
                             -a), but will print shell commands that export
                             environment variables (default names).  The result
                             for this option is the same as for using
                             'oidc-token -oie'. With the -o -i and -e options
                             the name of each environment variable can be
                             changed.
  -e, --expires-at[=OIDC_EXP]   Return the expiration time for the requested
                             access token. If neither -i nor -o is set and
                             OIDC_EXP is not passed, the expiration time is
                             printed to stdout. Otherwise shell commands are
                             printed that will export the value into an
                             environment variable. The name of this variable
                             can be set with OIDC_EXP.
  -i, --issuer[=OIDC_ISS]    Return the issuer associated with the requested
                             access token. If neither -e nor -o is set and
                             OIDC_ISS is not passed, the issuer is printed to
                             stdout. Otherwise shell commands are printed that
                             will export the value into an environment
                             variable. The name of this variable can be set
                             with OIDC_ISS.
      --no-seccomp           Disables seccomp system call filtering; allowing
                             all system calls. Use this option if you get an
                             'Bad system call' error and hand in a bug report.
  -o, --token[=OIDC_AT]      Return the requested access token. If neither -i
                             nor -e is set and OIDC_AT is not passed, the token
                             is printed to stdout (Same behaviour as without
                             this option). Otherwise shell commands are printed
                             that will export the value into an environment
                             variable. The name of this variable can be set
                             with OIDC_AT.
  -s, --scope=SCOPE          scope to be requested for the requested access
                             token. To provide multiple scopes, use this option
                             multiple times.

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <https://github.com/indigo-dc/oidc-agent/issues>.
```

<!-- TODO -->
