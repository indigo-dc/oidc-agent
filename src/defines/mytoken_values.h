#ifndef OIDC_AGENT_MYTOKEN_VALUES_H
#define OIDC_AGENT_MYTOKEN_VALUES_H

#define MYTOKEN_KEY_MYTOKEN_ENDPOINT "mytoken_endpoint"
#define MYTOKEN_KEY_PROVIDERS_SUPPORTED "providers_supported"
#define MYTOKEN_KEY_GRANTTYPES_SUPPORTED \
  "mytoken_endpoint_grant_types_supported"
#define MYTOKEN_KEY_POLLINGCODE "polling_code"
#define MYTOKEN_KEY_CONSENTURI "consent_uri"
#define MYTOKEN_KEY_MYTOKEN "mytoken"
#define MYTOKEN_KEY_TRANSFERCODE "transfer_code"
#define MYTOKEN_KEY_TOKENTYPE "token_type"
#define MYTOKEN_KEY_RESTRICTIONS "restrictions"
#define MYTOKEN_KEY_ROTATION "rotation"
#define MYTOKEN_KEY_CAPABILITIES "capabilities"

#define MYTOKEN_GRANTTYPE_OIDC "oidc_flow"
#define MYTOKEN_GRANTTYPE_POLLINGCODE "polling_code"
#define MYTOKEN_GRANTTYPE_MYTOKEN "mytoken"

#define MYTOKEN_MYTOKENTYPE_JWT "token"
#define MYTOKEN_MYTOKENTYPE_SHORTTOKEN "short_token"
#define MYTOKEN_MYTOKENTYPE_TRANSFERCODE "transfer_code"

#endif  // OIDC_AGENT_MYTOKEN_VALUES_H
