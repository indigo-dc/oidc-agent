## Known Issues

### Expiring Refresh Tokens

oidc-agent assumes that refresh tokens do not expire. But some providers might
use refresh tokens that expire after a certain time or when they are not used
for a specific time. To prevent the latter use oidc-agent / oidc-token regularly
(you can also use a cron job).

oidc-agent is able to
update a stored refresh token. However, therefore it has to receive a new
refresh token from the provider. If a refresh token expired (e.g. because the token was used within the lifetime of that
token), use `oidc-gen --reauthenticate <short_name>` to re-authenticate and update the refresh token.

