#ifndef OIDC_VALUES_H
#define OIDC_VALUES_H

// PROVIDER METADATA KEYS
#define OIDC_KEY_SCOPES_SUPPORTED "scopes_supported"
#define OIDC_KEY_GRANT_TYPES_SUPPORTED "grant_types_supported"
#define OIDC_KEY_RESPONSE_TYPES_SUPPORTED "response_types_supported"
#define OIDC_KEY_CODE_CHALLENGE_METHODS_SUPPORTED \
  "code_challenge_methods_supported"
// ENDPOINT KEYS
#define OIDC_KEY_TOKEN_ENDPOINT "token_endpoint"
#define OIDC_KEY_AUTHORIZATION_ENDPOINT "authorization_endpoint"
#define OIDC_KEY_REVOCATION_ENDPOINT "revocation_endpoint"
#define OIDC_KEY_REGISTRATION_ENDPOINT "registration_endpoint"
#define OIDC_KEY_DEVICE_AUTHORIZATION_ENDPOINT "device_authorization_endpoint"
#define OIDC_KEY_ISSUER "issuer"

// CLIENT KEYS
#define OIDC_KEY_REGISTRATION_CLIENT_URI "registration_client_uri"
#define OIDC_KEY_REGISTRATION_ACCESS_TOKEN "registration_access_token"

// TOKEN RESPONSE KEYS
#define OIDC_KEY_EXPIRESIN "expires_in"
#define OIDC_KEY_ACCESSTOKEN "access_token"
#define OIDC_KEY_REFRESHTOKEN "refresh_token"
#define OIDC_KEY_IDTOKEN "id_token"

// REQUEST KEYS
#define OIDC_KEY_CLIENTID "client_id"
#define OIDC_KEY_CLIENTSECRET "client_secret"
#define OIDC_KEY_GRANTTYPE "grant_type"
#define OIDC_KEY_RESPONSETYPE "response_type"
#define OIDC_KEY_SCOPE "scope"
#define OIDC_KEY_AUDIENCE "audience"
#define GOOGLE_KEY_ACCESSTYPE "access_type"
// AUTH CODE FLOW
#define OIDC_KEY_REDIRECTURI "redirect_uri"
#define OIDC_KEY_CODE "code"
#define OIDC_KEY_STATE "state"
#define OIDC_KEY_PROMPT "prompt"
#define OIDC_KEY_CODEVERIFIER "code_verifier"
#define OIDC_KEY_CODECHALLENGE "code_challenge"
#define OIDC_KEY_CODECHALLENGE_METHOD "code_challenge_method"
// PASSWORD FLOW
#define OIDC_KEY_USERNAME "username"
#define OIDC_KEY_PASSWORD "password"
// REVOCATION
#define OIDC_KEY_TOKENTYPE_HINT "token_type_hint"
#define OIDC_KEY_TOKEN "token"
// CLIENT REGISTRATION
#define OIDC_KEY_APPLICATIONTYPE "application_type"
#define OIDC_KEY_CLIENTNAME "client_name"
#define OIDC_KEY_GRANTTYPES "grant_types"
#define OIDC_KEY_RESPONSETYPES "response_types"
#define OIDC_KEY_REDIRECTURIS "redirect_uris"
// DEVICE FLOW
#define OIDC_KEY_DEVICECODE "device_code"
#define OIDC_KEY_USERCODE "user_code"
#define OIDC_KEY_VERIFICATIONURI "verification_uri"
#define GOOGLE_KEY_VERIFICATIONURI "verification_url"
#define OIDC_KEY_VERIFICATIONURI_COMPLETE "verification_uri_complete"
#define GOOGLE_KEY_VERIFICATIONURI_COMPLETE "verification_url_complete"
#define OIDC_KEY_INTERVAL "interval"
#define OIDC_SLOW_DOWN "slow_down"
#define OIDC_AUTHORIZATION_PENDING "authorization_pending"

// OIDC ERROR
#define OIDC_KEY_ERROR "error"
#define OIDC_KEY_ERROR_DESCRIPTION "error_description"

// GRANTTYPES
#define OIDC_GRANTTYPE_PASSWORD "password"
#define OIDC_GRANTTYPE_REFRESH "refresh_token"
#define OIDC_GRANTTYPE_AUTHCODE "authorization_code"
#define OIDC_GRANTTYPE_IMPLICIT "implicit"
#define OIDC_GRANTTYPE_DEVICE "urn:ietf:params:oauth:grant-type:device_code"
#define OIDC_PROVIDER_DEFAULT_GRANTTYPES \
  "[\"" OIDC_GRANTTYPE_AUTHCODE "\", \"" OIDC_GRANTTYPE_IMPLICIT "\"]"

// RESPONSETYPES
#define OIDC_RESPONSETYPE_TOKEN "token"
#define OIDC_RESPONSETYPE_CODE "code"

// APPLICATIONTYPES
#define OIDC_APPLICATIONTYPES_WEB "web"
#define OIDC_APPLICATIONTYPES_NATIVE "native"

// SCOPES
#define OIDC_SCOPE_OPENID "openid"
#define OIDC_SCOPE_OFFLINE_ACCESS "offline_access"

// TOKENTYPES
#define OIDC_TOKENTYPE_REFRESH "refresh_token"

// PROVIDER FIXES
#define GOOGLE_ISSUER_URL "https://accounts.google.com/"
#define GOOGLE_ACCESSTYPE_OFFLINE "offline"
#define ELIXIR_ISSUER_URL "https://login.elixir-czech.org/oidc/"
#define ELIXIR_SUPPORTED_SCOPES                                   \
  "openid offline_access profile email address phone groupNames " \
  "forwardedScopedAffiliations bona_fide_status country eduPersonEntitlement"
#define EDUTEAMS_ISSUER_URL "https://proxy.eduteams.org"

// PROMPT VALUES
#define OIDC_PROMPT_CONSENT "consent"

// PKCE
#define CODE_CHALLENGE_METHOD_PLAIN "plain"
#define CODE_CHALLENGE_METHOD_S256 "S256"
#define CODE_VERIFIER_LEN 128  // min: 43 max: 128

#endif  // OIDC_VALUES_H
