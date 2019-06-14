#ifndef AGENT_MAGIC_VALUES_H
#define AGENT_MAGIC_VALUES_H

#define AGENT_SCOPE_ALL "max"
#define AGENT_KEY_ISSUERURL "issuer_url"
#define AGENT_KEY_DAESETBYUSER "daeSetByUser"
#define AGENT_KEY_SHORTNAME "name"
#define AGENT_KEY_CERTPATH "cert_path"
#define AGENT_KEY_EXPIRESAT "expires_at"
#define AGENT_KEY_JWKS_SIGN "jwks_sign"
#define AGENT_KEY_JWKS_ENC "jwks_enc"
#define AGENT_KEY_ISSUER_JWKS_SIGN "iss_jwks_sign"
#define AGENT_KEY_ISSUER_JWKS_ENC "iss_jwks_enc"
#define AGENT_KEY_IDTOKEN_SIGN_ALG "id_token_signing_alg"
#define AGENT_KEY_IDTOKEN_ENC_ALG "id_token_encryption_alg"
#define AGENT_KEY_IDTOKEN_ENC_ENC "id_token_encryption_enc"
#define AGENT_KEY_USERINFO_SIGN_ALG "userinfo_signing_alg"
#define AGENT_KEY_USERINFO_ENC_ALG "userinfo_encryption_alg"
#define AGENT_KEY_USERINFO_ENC_ENC "userinfo_encryption_enc"
#define AGENT_KEY_REQUESTOBJECT_SIGN_ALG "request_object_signing_alg"
#define AGENT_KEY_REQUESTOBJECT_ENC_ALG "request_object_encryption_alg"
#define AGENT_KEY_REQUESTOBJECT_ENC_ENC "request_object_encryption_enc"
#define AGENT_KEY_JOSE_ENABLED "jose_enabled"

// INTERNAL / CLI FLOW VALUES
#define FLOW_VALUE_CODE "code"
#define FLOW_VALUE_PASSWORD "password"
#define FLOW_VALUE_DEVICE "device"
#define FLOW_VALUE_REFRESH "refresh"

#define AGENT_CUSTOM_SCHEME "edu.kit.data.oidc-agent:/"

#endif  // AGENT_MAGIC_VALUES_H
