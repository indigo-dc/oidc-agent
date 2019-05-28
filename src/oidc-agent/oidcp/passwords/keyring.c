#ifndef __APPLE__
#include "keyring.h"
#include "utils/logger.h"
#include "utils/oidc_error.h"

#include <libsecret/secret.h>

const SecretSchema* agent_get_schema(void) G_GNUC_CONST;

#define AGENT_SCHEMA agent_get_schema()

const SecretSchema* agent_get_schema(void) {
  static const SecretSchema the_schema = {
      "edu.kit.oidc-agent.Password",
      SECRET_SCHEMA_NONE,
      {
          {"shortname", SECRET_SCHEMA_ATTRIBUTE_STRING},
          {"NULL", 0},
      },
      // These are just reserved variables
      0,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL};
  return &the_schema;
}

void oidc_setGerror(GError* error) {
  if (error == NULL) {
    return;
  }
  logger(ERROR, "%s", error->message);
  oidc_errno = OIDC_EGERROR;
  oidc_seterror(error->message);
}

char* keyring_getPasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return NULL;
  }
  logger(DEBUG, "Looking up password for '%s' in keyring", shortname);
  GError* error = NULL;
  gchar*  pw    = secret_password_lookup_sync(AGENT_SCHEMA, NULL, &error,
                                          "shortname", shortname, NULL);
  if (error != NULL) {
    oidc_setGerror(error);
    g_error_free(error);
    return NULL;
  }
  if (pw == NULL) {
    logger(DEBUG, "No password found for '%s' in keyring", shortname);
    oidc_errno = OIDC_EPWNOTFOUND;
    return NULL;
  }
  char* ret = oidc_strcopy(pw);
  secret_password_free(pw);
  return ret;
}

oidc_error_t keyring_savePasswordFor(const char* shortname,
                                     const char* password) {
  if (shortname == NULL || password == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  logger(DEBUG, "Saving password for '%s' in keyring", shortname);
  GError* error = NULL;
  secret_password_store_sync(AGENT_SCHEMA, SECRET_COLLECTION_DEFAULT, shortname,
                             password, NULL, &error, "shortname", shortname,
                             NULL);

  if (error == NULL) {
    logger(DEBUG, "Password for '%s' saved in keyring", shortname);
    return OIDC_SUCCESS;
  }
  oidc_setGerror(error);
  g_error_free(error);
  return oidc_errno;
}

oidc_error_t keyring_removePasswordFor(const char* shortname) {
  if (shortname == NULL) {
    oidc_setArgNullFuncError(__func__);
    return oidc_errno;
  }
  logger(DEBUG, "Removing password for '%s' from keyring", shortname);
  GError*  error   = NULL;
  gboolean removed = secret_password_clear_sync(AGENT_SCHEMA, NULL, &error,
                                                "shortname", shortname, NULL);

  if (error != NULL) {
    oidc_setGerror(error);
    g_error_free(error);
    return oidc_errno;
  }
  if (removed) {
    logger(DEBUG, "Password for '%s' removed from keyring", shortname);
  } else {
    logger(DEBUG, "No password to remove for '%s' in keyring", shortname);
  }
  return OIDC_SUCCESS;
}
#endif
