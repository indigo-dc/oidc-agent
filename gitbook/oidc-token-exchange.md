# oidc-token-exchange
oidc-token-exchange is <!-- TODO -->

```
$ oidc-token-exchange --help
Usage: oidc-token-exchange [OPTION...]
            SHORT_NAME [ISSUER_URL CLIENT_ID CLIENT_SECRET ACCESS_TOKEN]
oidc-token-exchange -- A tool for performing OIDC token exchanges using
oidc-agent

 General Usage:
  -r, --revoke               Removes an account from the agent and revokes the
                             associated refresh token.
  -t, --lifetime=LIFETIME    Set a maximum lifetime in seconds when adding the
                             account configuration

 Advanced:
      --cp=CERT_PATH         CERT_PATH is the path to a CA bundle file that
                             will be used with TLS communication
      --no-seccomp           Disables seccomp system call filtering; allowing
                             all system calls. Use this option if you get an
                             'Bad system call' error and hand in a bug report.

 Verbosity:
  -g, --debug                Sets the log level to DEBUG
  -v, --verbose              Enables verbose mode

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <https://github.com/indigo-dc/oidc-agent/issues>.
```

<!-- TODO -->
