#define _GNU_SOURCE
#include "gettext.h"

#include "utils/string/stringUtils.h"
#include "utils/prompt_mode.h"

char* gettext(const char* msgid) {

    if (msgid=="update-account-config") { 
        // oidc-agent/oidcp/passwords/askpass.c:18
        // completed with: // formatstring used outside
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Update Account Config</h2>"
                "<p/>oidc-agent needs to update the account config for <b>%s</b>.\nPlease enter "
                "the encryption password for <b>%s</b>:";
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "oidc-agent needs to update the account config for '%s'.\nPlease enter "
                "the encryption password for '%s':";
            );
        }
    }

    if (msgid=="unlock-account-config") || (msgid=="unlock-account-config-autoload") { 
        // oidc-agent/oidcp/passwords/askpass.c:38 and 64
        // careful: I've removed one shortname in relation to upstream commit
        // completed with: ();
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Unlock Identity</h2>"
                "<p/><b>%s</b>requests an access token for <b>%s</b>.\n"
                "Enter password to unlock account <b>%s</b> for this identity.";
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "%srequests an access token for '%s'.\nThis configuration "
                "is currently not loaded.\nTo load '%s' into oidc-agent please enter "
                "the encryption password for '%s':";
            );
        }
    }

    if (msgid=="confirm-iss-to-use-config") { 
        // oidc-agent/oidcp/passwords/askpass.c:86
        // completed with: ();
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Confirm</h2>"
                "<p/><b>%s</b>requests an access token for <b>%s</b>.\n"
                "Do you want to allow this usage?";
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "%srequests an access token for '%s'.\n"
            );
        }
    }

    if (msgid=="confirm-iss-to-use-config-with-iss") { 
        // oidc-agent/oidcp/passwords/askpass.c:109
        // completed with: ();
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Confirm</h2>"
                "<p/><b>%s</b>requests an access token for <b>%s</b>.\n"
                "Do you want to allow the usage of <b>%s</b>?";
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "%srequests an access token for '%s'.\n"
                "Do you want to allow the usage of '%s'?";
            );
        }
    }

    if (msgid=="confirm-id-token") { 
        // 
        // oidc-agent/oidcp/passwords/askpass.c
        // completed with: ();
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Confirm</h2>"
                "<p/><b>%s</b>requests an id token for <b>%s</b>.\n"
                "id tokens should not be passed to other applications as authorization.\n"
                "Do you want to allow this usage?";
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "An application %srequests an id token for '%s'.\n"
            );
        }
    }

    if (msgid=="confirm-id-token-with-iss") { 
        // 
        // oidc-agent/oidcp/passwords/askpass.c
        // completed with: ();
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Confirm</h2>"
                "<b>%s</b>requests an id token for <b>%s</b>.\n"
                "id tokens should not be passed to other applications as authorization.\n"
                "Do you want to allow the usage of <b>%s</b>?";
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "An application %srequests an id token for '%s'.\n"
                "Do you want to allow the usage of '%s'?";
            );
        }
    }



    if (msgid=="device-code-at-url") { 
        // from oidc-agent/oidcp/passwords/agent_prompt.c:58
        // completed with: (intro, qr_part, code_part);
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Authenticate</h2>"
                "%s"
                "<p/>To continue please open the following URL in a browser on any device "
                "%s%s"
                "<p class=\"tiny\">You need to close this window manually</p>\n"
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "%sTo continue please open the following URL in a browser on any device "
                "%s%s"
            );
        }
    }

    if (msgid=="device-code-at-url") { 
        // from oidc-agent/oidcp/passwords/agent_prompt.c:70
        // completed with: (intro);
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Authenticate</h2>%s"
                "<p/>To continue please open the following URL in your browser:\n"
                "<p class=\"tiny\">You need to close this window manually</p>\n"
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "%sTo continue please open the following URL in your browser:\n"
            );
        }
    }



    if (msgid=="enter-shortname") { 
        // from oidc-gen/gen_handler.c:359
        // completed with: ();
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Configure account</h2><p/>Enter short name for the account to configure: "
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "Enter short name for the account to configure:"
            );
        }
    }



    if (msgid=="enter-other-shortname") { 
        // from oidc-gen/promptAndSet/name.c:24
        // completed with: ("exists");
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Configure account</h2><p/>%s<p/>"
                "Enter short name for the account to configure",
                exists ? "An account with that shortname is already "
                         "configured.\nPlease choose another name.\n\n"
                       : "");
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                ""
            );
        }
    }



    if (msgid=="enter-password-for") { 
        // utils/promptUtils.c:70
        // completed with: (forWhat);
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Enter Password</h2><p/>Enter encryption password for %s", forWhat);
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "Enter encryption password for %s"
            );
        }
    }

    if (msgid=="confirm-password") { 
        // utils/promptUtils.c:81
        // completed with: ();
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                "<h2>Confirm Password</h2><p/>Confirm encryption Password", "Encryption password",
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                "Confirm encryption Password"
            );
        }
    }

    if (msgid=="encryption-password") { 
        // utils/promptUtils.c:71 and 83
        // completed with: ();
            text = oidc_vsprintf(
                "Encryption password"
            );
    }

    if (msgid=="passwords-dont-match") { 
        // utils/promptUtils.c:86
        // completed with: ();
            text = oidc_vsprintf(
                "Encryption passwords did not match.\n"
            );
    }



    if (msgid=="link-identity") { 
        // oidc-agent/oidcp/oidcp.c:571
        // completed with:  application_str ?: "", issuer);
        if (prompt_mode() == PROMPT_MODE_GUI) {
            text = oidc_vsprintf(
                    "<h2>Link Identity</h2>"
                    "<p/>Application <b>%s</b>requests an access token for <b>%s</b>. "
                    "<p/>There currently is no identity configured for this "
                    "issuer. Do you want configure one now?",
                    application_str ?: "", issuer;
            );
        }
        if (prompt_mode() == PROMPT_MODE_CLI) {
            text = oidc_vsprintf(
                    "An application %srequests an access token for '%s'. "
                    "There currently is no account configured for this "
                    "issuer. Do you want to automatically create one?",
                    application_str ?: "", issuer;
            );
        }
    }
}
