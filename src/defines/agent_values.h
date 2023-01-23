#ifndef AGENT_MAGIC_VALUES_H
#define AGENT_MAGIC_VALUES_H

#define AGENT_SCOPE_ALL "max"
#define AGENT_KEY_ISSUERURL "issuer_url"
#define AGENT_KEY_DAESETBYUSER "daeSetByUser"
#define AGENT_KEY_CONFIG_ENDPOINT "config_endpoint"
#define AGENT_KEY_SHORTNAME "name"
#define AGENT_KEY_CERTPATH "cert_path"
#define AGENT_KEY_EXPIRESAT "expires_at"
#define AGENT_KEY_OAUTH "oauth"
#define AGENT_KEY_MYTOKENURL "mytoken_url"
#define AGENT_KEY_MYTOKENPROFILE "mytoken_profile"
#define AGENT_KEY_PUBCLIENT "pubclient"
#define AGENT_KEY_CONTACT "contact"
#define AGENT_KEY_MANUAL_CLIENT_REGISTRATION_URI "register"
#define AGENT_KEY_PWSTORE "pw-store"
#define AGENT_KEY_DEFAULT_ACCOUNT "default_account"
#define AGENT_KEY_ACCOUNTS "accounts"

#define CONFIG_KEY_AGENT "oidc-agent"
#define CONFIG_KEY_ADD "oidc-add"
#define CONFIG_KEY_GEN "oidc-gen"
#define CONFIG_KEY_CLIENT "oidc-token"
#define CONFIG_KEY_BINDADDRESS "bind_address"
#define CONFIG_KEY_CONFIRM "confirm"
#define CONFIG_KEY_AUTOLOAD "autoload"
#define CONFIG_KEY_AUTOGEN "auto-gen"
#define CONFIG_KEY_AUTOREAUTH "auto-reauthenticate"
#define CONFIG_KEY_WEBSERVER "webserver"
#define CONFIG_KEY_CUSTOMURISCHEME "custom-uri-scheme"
#define CONFIG_KEY_DEBUGLOGGING "debug_logging"
#define CONFIG_KEY_GROUP "group"
#define CONFIG_KEY_CNID "cnid"
#define CONFIG_KEY_AUTOOPENURL "auto-open-url"
#define CONFIG_KEY_DEFAULTGPGKEY "default_gpg_key"
#define CONFIG_KEY_PROMPTMODE "prompt"
#define CONFIG_KEY_PWPROMPTMODE "pw-prompt"
#define CONFIG_KEY_ANSWERCONFIRMPROMPTS "answer-confirm-prompts"
#define CONFIG_KEY_DEFAULTMYTOKENSERVER "default_mytoken_server"
#define CONFIG_KEY_DEFAULTMYTOKENPROFILE "default_mytoken_profile"
#define CONFIG_KEY_PREFERMYTOKENOVEROIDC "prefer_mytoken_over_oidc"
#define CONFIG_KEY_STOREPW "store-pw"
#define CONFIG_KEY_DEFAULTMINLIFETIME "default-min-lifetime"

// INTERNAL / CLI FLOW VALUES
#define FLOW_VALUE_CODE "code"
#define FLOW_VALUE_PASSWORD "password"
#define FLOW_VALUE_DEVICE "device"
#define FLOW_VALUE_REFRESH "refresh"
#define FLOW_VALUE_MT_OIDC "mt_oidc"

#define AGENT_CUSTOM_SCHEME "edu.kit.data.oidc-agent:/"

#define FORCE_NEW_TOKEN -1

#endif  // AGENT_MAGIC_VALUES_H
