## Agent Locking

The agent can be locked using a locking password. While being locked the only
operations allowed are:

- checking if the agent is running
- unlocking the agent
  Every other request will result in an error `Agent locked`. This allows a
  user to temporarily forbid all operations / requests without removing the loaded
  account configurations.

While the agent is locked, refresh tokens, access tokens, and client credentials are encrypted using the locking
password provided by the user.

The agent also offers brute force protection. When trying to unlock the agent
with a wrong password a small delay is added, which will increase with the
number of failed attempts up to 10 seconds.


