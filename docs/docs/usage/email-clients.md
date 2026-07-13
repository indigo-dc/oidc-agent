# Using oidc-agent with Email Clients

Many email clients support running an external command to obtain
authentication credentials. Because `oidc-token` prints an access token to
`stdout`, it can be used directly as that external command — no wrapper
scripts, no manual token pasting, and tokens are refreshed automatically by
`oidc-agent`.

This page covers the setup for two popular command-line email tools:

- [Mutt / NeoMutt](#mutt--neomutt) — a terminal-based email client
- [mbsync (isync)](#mbsync-isync) — an IMAP mailbox synchronization tool

## Prerequisites

Before configuring any email client, make sure that:

1. `oidc-agent` is running (see [Starting oidc-agent](oidc-agent/start.md)).
2. You have an account configuration for your email provider (see
   [oidc-gen](oidc-gen/index.md)). When running `oidc-gen`, request the
   scopes your provider requires for mail access (e.g.
   `https://mail.google.com/` for Gmail).
3. You can successfully obtain a token on the command line:

```
oidc-token <shortname>
```

Replace `<shortname>` with the name you chose when creating the account
configuration (e.g. `my-email`).

## Mutt / NeoMutt

[Mutt](http://www.mutt.org/) and [NeoMutt](https://neomutt.org/) have
built-in support for OAUTHBEARER and XOAUTH2 authentication. They can run
an external command to obtain a fresh access token whenever one is needed.
This is the only email-client-specific configuration required — add the
following lines to your `~/.muttrc` (or `~/.config/mutt/muttrc`):

```
# OAuth2 authentication via oidc-agent
set imap_authenticators = "oauthbearer:xoauth2"
set imap_oauth_refresh_command = "oidc-token my-email"

set smtp_authenticators = "oauthbearer:xoauth2"
set smtp_oauth_refresh_command = "oidc-token my-email"
```

| Setting                        | Purpose                                                                |
|--------------------------------|------------------------------------------------------------------------|
| `imap_authenticators`          | Use OAUTHBEARER or XOAUTH2 for IMAP instead of plain password auth.    |
| `imap_oauth_refresh_command`   | Command that prints a fresh access token to stdout for IMAP.           |
| `smtp_authenticators`          | Same as above, but for outgoing mail (SMTP).                           |
| `smtp_oauth_refresh_command`   | Command that prints a fresh access token to stdout for SMTP.           |

Every time Mutt needs to authenticate it executes the configured command.
`oidc-token` contacts the running `oidc-agent`, which returns a valid access
token (refreshing it transparently if needed).

For all other Mutt settings (IMAP/SMTP server addresses, TLS, folders, etc.)
refer to the [Mutt manual](http://www.mutt.org/doc/manual/) or
[NeoMutt documentation](https://neomutt.org/guide/).

## mbsync (isync)

[mbsync](https://isync.sourceforge.io/) synchronizes IMAP mailboxes to local
Maildirs. It supports the `PassCmd` option, which runs a shell command and
uses its output as the authentication credential.

### Install the XOAUTH2 SASL plugin

mbsync uses the [Cyrus SASL](https://www.cyrusimap.org/sasl/) library for
authentication. The XOAUTH2 mechanism is **not** included by default — you
need to install a SASL plugin that provides it:

```
sudo apt install libsasl2-modules-kdexoauth2
```

If this plugin is missing, mbsync will fail with
*"Selected SASL mechanism not available"*.

### Configuration

In the `IMAPAccount` section of your `~/.mbsyncrc`, add the following
OAuth2-related settings:

```
IMAPAccount my-email
# (Your Host, User, and SSL settings go here)

PassCmd "oidc-token my-email"
AuthMechs XOAUTH2

# (Your IMAPStore, MaildirStore, and Channel settings go below)
```

| Setting     | Purpose                                                                 |
|-------------|-------------------------------------------------------------------------|
| `PassCmd`   | Runs `oidc-token my-email` and uses its stdout as the token.            |
| `AuthMechs` | Restricts authentication to XOAUTH2 so the token is sent as an OAuth2 credential. |

Each time mbsync connects it executes the `PassCmd`. `oidc-token` contacts
the running `oidc-agent`, which returns a valid access token (refreshing it
transparently if needed).

For the full `~/.mbsyncrc` structure (IMAPStore, MaildirStore, Channel, etc.)
refer to the [mbsync documentation](https://isync.sourceforge.io/mbsync.html).
