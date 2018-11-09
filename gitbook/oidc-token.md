# oidc-token
oidc-token is an example agent client using the provided C-API and can be used to 
easily get an OIDC access token from the command line. 

```
$ oidc-token --help
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
export ACCESS_TOKEN=`oidc-token <shortname>`
```

Alternatively the name of the environment variable can be passed to the ```-o```
option. This will print out shell commands that will set the environment
variable. Using ```eval``` they can automatically be called:
```
eval `oidc-token <shortname> -o ACCESS_TOKEN`
```

## Information Available from oidc-token
oidc-token cannot only provide an access token, but also the issuer url of the
issuer for which the access token is valid. This information might be required
by other applications. Additionally the time when the token expires can also be
returned.
There are multiple ways to obtain all information from oidc-token:
- Make multiple calls to oidc-token: Get each piece of information from one
  call:
  - ```oidc-token <shortname> [-o]``` to get the access token
  - ```oidc-token <shortname> -i``` to get the issuer url
  - ```oidc-token <shortname> -e``` to get the expiration time
  
  However this way is **not** recommended. This will make three independet token
  requests to oidc-agent. This is not only inefficient but also not guranteed to
  return correct results. It might happen that the token requested in the first
  call is only valid for a very short time and not valid anymore when doing the
  last request; in this case a new token will be requested that has a different
  expiration time that does not relate to the token from the first call.
- Use the ```-a``` option to get all information: oidc-token will print all
  information to stdout. One piece of information per line:
  - First line: access token
  - Second line: issuer url
  - Third line: expiration time
- Use environment variables: Using the ```-c``` option oidc-token will print out
  shell commands that can be evaluated to set environment variables (name of the
  environment variables are defaults):
  - ```OIDC_AT```: access token
  - ```OIDC_ISS```: issuer url
  - ```OIDC_EXP```: expiration date
  
  ```eval `oidc-token <short_name> -c` ``` will automatically set these
  variables. Using the ```-o```, ```-i```, and ```-e``` option the name of the
  exported variables can be customized. 


## oidc-token and Scopes
The ```--scope``` flag can be used to specify specific scopes. The returned
access token will be only valid for these scope values. The flag takes one scope, but multiple scopes can be passed by using this options multiple times. All passed scope values have to be registered for this client.

If the flag is not provided the default scope is used.

# Other agent clients
Any application that needs an access token can use our API to get an access token from 
oidc-agent. The following applications are already able to get an access token from oidc-agent:
- [wattson](https://github.com/indigo-dc/wattson)
- [orchent](https://github.com/indigo-dc/orchent)
