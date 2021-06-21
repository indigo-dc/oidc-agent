# oidc-agent in user-space
While `oidc-agent` itself does not root privileges to run, it must be installed
and also its dependencies.
This is not easily doable if one does not have root privileges to do so.

However, it is possible to use `oidc-agent` in user space without needing root
privileges through the use of a docker image and [`udocker`](https://github.com/indigo-dc/udocker).

## Installation and Setup
First you need to install `udocker`. This is also doable without root
privileges. Please refer to their [installation
manual](https://indigo-dc.github.io/udocker/installation_manual.html) to install
`udocker`.

Then download the docker image and create the udocker container:
```
udocker pull myoidc/oidc-agent
udocker create --name=oidc-agent myoidc/agent
```

If you later want to update your `oidc-agent`. You have to do this steps:
```
udocker rm oidc-agent
udocker pull myoidc/oidc-agent
udocker create --name=oidc-agent myoidc/agent
```

Now we create a oidc-agent directory on your host system to store the
configurations persistently:
```
mkdir $HOME/.oidc-agent
```

Create aliases for the long commands. Add the following to your `.bash_aliases`:
```
alias oidc-agent="nohup udocker -q run -p 4242:4242 -p 8080:8080 -v $HOME/.oidc-agent:/home/agent/.oidc-agent -v /tmp:/tmp oidc-agent >/tmp/udocker-oidc-agent.log 2>&1 &"
alias oidc-gen="udocker -q run -v $HOME/.oidc-agent:/home/agent/.oidc-agent -v /tmp:/tmp oidc-agent oidc-gen"
alias oidc-add="udocker -q run -v $HOME/.oidc-agent:/home/agent/.oidc-agent -v /tmp:/tmp oidc-agent oidc-add"
alias oidc-token="udocker -q run -v $HOME/.oidc-agent:/home/agent/.oidc-agent -v /tmp:/tmp oidc-agent oidc-token"
alias oidc-agent-status="udocker -q run -v $HOME/.oidc-agent:/home/agent/.oidc-agent -v /tmp:/tmp oidc-agent oidc-agent --status"
```

## Usage
If you completed all the previous steps you can use `oidc-agent` with the
following commands:
```
oidc-agent    # Starts the agent in the background, only run this once.
oidc-gen      # To create an account configuration
oidc-add      # To load and unload accounts
oidc-token    # To obtain access tokens
```
For more information please refer to the [documentation](/user). <!--TODO-->


If other applications on your host machine want to connect to the agent, this is
also possible. You only must export the `OIDC_SOCK`:
```
export OIDC_SOCK=/tmp/oidc-agent-service/1000/oidc-agent.sock
```

## Drawbacks
Compared to a native installation the this variant has the following small
drawbacks:
- No GUI prompting is possible. This means that the following is not possible:
  - Autoload: Accounts cannot be loaded automatically if needed, but must be
      loaded explicitly with `oidc-add` before they can be used.
  - GUI Password prompting. On some providers refresh tokens might change and in
      order to `oidc-agent` be able to store an updated refresh token, it needs
      the encryption password. Using this approach `oidc-agent` cannot prompt
      you for it when needed. However, if you know your provider is effected,
      you can store the password in the agent when loading the account
      configuration by adding the `--store-pw` option to `oidc-add`.
  - GUI prompting for `oidc-gen` and `oidc-add`
- `oidc-gen` cannot automatically open your browser: You have to manually copy
    and paste the URL from `oidc-gen` when instructed.
- Custom URI schemes are not supported.
