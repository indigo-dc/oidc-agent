#ifndef OIDC_VERSION_H
#define OIDC_VERSION_H

#ifndef VERSION
#define VERSION #include "VERSION"
#endif  // VERSION
#define BUG_ADDRESS                                                     \
  "<https://github.com/indigo-dc/oidc-agent/issues>\nSubscribe to our " \
  "mailing list to receive important updates about oidc-agent: "        \
  "<https://www.lists.kit.edu/sympa/subscribe/oidc-agent-user>"
#define AGENT_VERSION "oidc-agent " VERSION
#define GEN_VERSION "oidc-gen " VERSION
#define ADD_VERSION "oidc-add " VERSION
#define TOKEN_VERSION "oidc-token " VERSION

#define PROMPT_VERSION "2.0.0"

#endif  // OIDC_VERSION_H
