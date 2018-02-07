# User Guide
<!-- ## Quickstart for INDIGO IAM -->
<!-- Once installed a account configuration for INDIGO IAM can be generated.  -->
<!--  -->
<!-- The next lines will start oidc-agent and the client registration process. -->
<!-- ``` -->
<!-- eval `oidc-agent` -->
<!-- oidc-gen <shortname> -->
<!-- ``` -->
<!--  -->
<!-- A client will be registered, but it will lack permission to use the -->
<!-- password grant type.  Send the client-id displayed by oidc-gen to an -->
<!-- INDIGO IAM admin (Andrea) to update the client configuration.  -->
<!--  -->
<!-- After the client configuration was updated by an admin, the account -->
<!-- configuration generation can be finished.  For this you need to provide -->
<!-- the clientconfig file generated during the previous call to oidc-gen to do -->
<!-- so. (Client configs are stored in <$HOME/.config/oidc-agent>.clientconfig) -->
<!--  -->
<!-- ``` -->
<!-- oidc-gen -f <path_to_clientconfigfile> -->
<!-- ``` -->
<!-- Now the account configuration was created and already added to the agent.  -->
<!--  -->
<!-- Test it with: -->
<!-- ``` -->
<!-- oidc-token <shortname> -->
<!-- ``` -->
<!--  -->
<!-- After this initial generation the account configuration can be added to the started agent using oidc-add: -->
<!-- ``` -->
<!-- oidc-add <shortname> -->
<!-- ``` -->

Using oidc-agent is made as easy as possible. In case you are lost oidc-agent and relating components provide
a lot of information with their 'help' command, just call `oidc-agent --help` or
refer to this documentation.

If you need help with a specific provider please refer to [this
section](provider.md).
