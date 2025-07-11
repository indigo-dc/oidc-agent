# General Configuration

Starting with oidc-agent `5.0.0` oidc-agent now has a central configuration file. Before, (apart from some exceptions)
it was only possible to use command line options to tweak behavior.
With the new configuration file a lot of the options can be set globally.

The `config` file can be located in `/etc/oidc-agent` or the oidc-agent directory. If both files are present the
configurations are merged, where options from the user's oidc-agent directory overwrite options specified in the global
config file.
The file is structured into sections for the different tools and configuration options should be self-explaining or
explained in the commented default configuration file.