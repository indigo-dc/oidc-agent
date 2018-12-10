# oidc-token-exchange
oidc-token-exchange is a tool to perform token exchanges. It can be sued to
obtain a refresh token from an valid access token. However, this requires a
special, preregisters client that has the token exchange grant type enabled.
One use case where this tool can be used is getting additional access tokens for
long running computing jobs.

Using ```oidc-token-exchange <shortname> <issuer_url> <client_id>
<client_secret> <access token>``` a configuration can be added with only these
information provided. This can be easily done inside a script. 

Then access tokens can be requested using ```oidc-token-exchange <shortname>```
or using ```oidc-token```.

After usage (e.g. job done) the refresh token has to be revoked. To do this call
```oidc-token-exchange -r <shortname>```. This will revoke the refresh token and
unload the configuration from the agent.

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
      --cp=FILE              FILE is the path to a CA bundle file that will be
                             used with TLS communication
  -p, --persist              The generated account configuration is persisted.
                             This means it can be loaded and unloaded using
                             oidc-add. Do NOT use oidc-token-exchange -r to
                             delete a persistent configuration; use oidc-gen -d
                             instead.
      --seccomp              Enables seccomp system call filtering; allowing
                             only predefined system calls.

 Verbosity:
  -g, --debug                Sets the log level to DEBUG
  -v, --verbose              Enables verbose mode

 Help:
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <https://github.com/indigo-dc/oidc-agent/issues>
Subscribe to our mailing list to receive important updates about oidc-agent:
<https://www.lists.kit.edu/sympa/subscribe/oidc-agent-user>.
```

