#define _XOPEN_SOURCE 700

#include "prompt.h"

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "memory.h"
#include "oidc_error.h"
#include "printer.h"
#include "utils/file_io/file_io.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/prompt_mode.h"
#include "utils/string/stringUtils.h"
#include "utils/system_runner.h"

#define OIDC_PROMPT "oidc-prompt"

#define OIDC_PROMPT_FMT OIDC_PROMPT " %s \"%s\" \"%s\" \"%s\" \"%d\" \"%s\""

char* oidcPromptCmd(const char* type, const char* title, const char* text,
                    const char* label, const char* init, const int timeout) {
  return oidc_sprintf(OIDC_PROMPT_FMT, type, title, text, label, timeout, init);
}

char* oidcPromptCmdWithList(const char* type, const char* title,
                            const char* text, const char* label, list_t* inits,
                            size_t initPos, const int timeout) {
  if (initPos >= inits->len) {
    initPos = 0;
  }
  const char* init = inits->len > 0 ? list_ats(inits, initPos)->val : "";
  char*       cmd  = oidcPromptCmd(type, title, text, label, init, timeout);
  for (size_t i = 0; i < inits->len; i++) {
    if (i == initPos) {
      continue;
    }
    char* tmp = oidc_sprintf("%s \"%s\"", cmd, (char*)list_ats(inits, i)->val);
    if (tmp == NULL) {
      logger(ERROR, oidc_serror());
    } else {
      secFree(cmd);
      cmd = tmp;
    }
  }
  return cmd;
}

void displayLinkGUI(const char* text, const char* link, const char* qr_path) {
  char* cmd = oidcPromptCmd("link", "oidc-agent - Reauthentication required",
                            text, link, qr_path ?: "", PROMPT_DEFAULT_TIMEOUT);
  fireCommand(cmd);
  secFree(cmd);
}

char* _promptPasswordGUI(const char* text, const char* label, const char* init,
                         const int timeout) {
  char* cmd = oidcPromptCmd("password", "oidc-agent password prompt", text,
                            label, init ?: "", timeout);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  if (!strValid(ret)) {  // Cancel
    raise(SIGINT);       // Cancel should have the same behaviour as interrupt
  }
  return ret;
}

char* _promptGUI(const char* text, const char* label, const char* init,
                 const int timeout) {
  char* cmd = oidcPromptCmd("input", "oidc-agent prompt", text, label,
                            init ?: "", timeout);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  if (!strValid(ret)) {  // Cancel
    raise(SIGINT);       // Cancel should have the same behaviour as interrupt
  }
  return ret;
}

char* _promptSelectGUI(const char* text, const char* label, list_t* init,
                       size_t initPos, const int timeout) {
  char* cmd = oidcPromptCmdWithList("select-other", "oidc-agent prompt", text,
                                    label, init, initPos, timeout);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  if (!strValid(ret)) {  // Cancel
    raise(SIGINT);       // Cancel should have the same behaviour as interrupt
  }
  return ret;
}

list_t* _promptMultipleGUI(const char* text, const char* label, list_t* init,
                           const int timeout) {
  char* text_p = oidc_sprintf("%s (One value per line)", text);
  char* cmd    = oidcPromptCmdWithList("multiple", "oidc-agent prompt", text_p,
                                       label, init, 0, timeout);
  secFree(text_p);
  char* input = getOutputFromCommand(cmd);
  secFree(cmd);
  if (!strValid(input)) {  // Cancel
    raise(SIGINT);         // Cancel should have the same behaviour as interrupt
  }
  list_t* out = delimitedStringToList(input, '\n');
  secFree(input);
  return out;
}

char* _promptCLI(const char* text, const char* init) {
  printPrompt("%s", text);
  if (init) {
    printPrompt(" [%s]", init);
  }
  printPrompt(": ");
  char* input = getLineFromFILE(stdin);
  if (input && strequal(input, "") && init) {
    secFree(input);
    return oidc_strcopy(init);
  }
  return input;
}

char* _promptPasswordCLI(const char* text, const char* init) {
  struct termios oflags, nflags;

  /* disabling echo */
  tcgetattr(STDIN_FILENO, &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &nflags) != 0) {
    logger(ERROR, "tcsetattr: %m");
    oidc_errno = OIDC_ETCS;
    return NULL;
  }

  char* password = _promptCLI(text, init ? "***" : NULL);

  /* restore terminal */
  if (tcsetattr(STDIN_FILENO, TCSANOW, &oflags) != 0) {
    logger(ERROR, "tcsetattr: %m");
    oidc_errno = OIDC_ETCS;
    secFree(password);
    return NULL;
  }

  if (password && strequal(password, "***") && init) {
    secFree(password);
    password = oidc_strcopy(init);
  }
  return password;
}

void _printSelectOptions(list_t* suggastable) {
  if (suggastable == NULL) {
    return;
  }
  size_t i;
  for (i = 0; i < suggastable->len;
       i++) {  // printed indices starts at 1 for non nerd
    printPrompt("[%lu] %s\n", i + 1, (char*)list_ats(suggastable, i)->val);
  }
}

char* _promptSelectCLI(const char* text, list_t* init, size_t initPos) {
  char* fav = NULL;
  if (init == NULL || list_ats(init, initPos) == NULL) {
    logger(ERROR, "In %s initPos not valid", __func__);
  } else {
    fav = list_ats(init, initPos)->val;
  }
  char* out = NULL;
  _printSelectOptions(init);
  while (out == NULL) {
    char* input = _promptCLI(text, fav);
    if (!strValid(input)) {
      out = oidc_strcopy(fav);
      secFree(input);
      return out;
    } else if (isdigit(*input)) {
      size_t i = strToULong(input);
      secFree(input);
      if (i == 0 || i > init->len) {
        printError("Input out of bound\n");
        continue;
      }
      i--;  // printed indices start at 1 for non nerds
      out = oidc_strcopy(list_ats(init, i)->val);
    } else {
      out = input;
    }
  }
  return out;
}

list_t* _promptMultipleCLI(const char* text, list_t* init) {
  char* arr_str = listToDelimitedString(init, " ");
  char* text_p  = oidc_sprintf("%s (space separated)", text);
  char* input   = _promptCLI(text_p, arr_str);
  secFree(arr_str);
  secFree(text_p);
  list_t* ret = delimitedStringToList(input, ' ');
  secFree(input);
  return ret;
}

/**
 * @brief prompts the user and disables terminal echo for the userinput, so it
 * is useable for password prompts
 * @param prompt_mode the mode how the user should be prompted
 * @return a pointer to the user input. Has to be freed after usage.
 */
char* promptPassword(const char* text, const char* label, const char* init,
                     unsigned char cliVerbose) {
  char* password = NULL;
  switch (pw_prompt_mode()) {
    case PROMPT_MODE_CLI:
      password = _promptPasswordCLI(cliVerbose ? text : label, init);
      break;
    case PROMPT_MODE_GUI:
      password = _promptPasswordGUI(text, label, init, PROMPT_NO_TIMEOUT);
      break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return password;
}

/**
 * @brief prompts the user for userinput, should not be used for password
 * prompts, user \f promptPassword instead
 * @param prompt_mode the mode how the user should be prompted
 * @return a pointer to the user input. Has to freed after usage.
 */
char* prompt(const char* text, const char* label, const char* init,
             unsigned char cliVerbose) {
  char* input = NULL;
  switch (prompt_mode()) {
    case PROMPT_MODE_CLI:
      input = _promptCLI(cliVerbose ? text : label, init);
      break;
    case PROMPT_MODE_GUI:
      input = _promptGUI(text, label, init, PROMPT_NO_TIMEOUT);
      break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return input;
}

char* promptSelect(const char* text, const char* label, list_t* options,
                   size_t initPos, unsigned char cliVerbose) {
  char* input = NULL;
  switch (prompt_mode()) {
    case PROMPT_MODE_CLI:
      input = _promptSelectCLI(cliVerbose ? text : label, options, initPos);
      break;
    case PROMPT_MODE_GUI:
      input =
          _promptSelectGUI(text, label, options, initPos, PROMPT_NO_TIMEOUT);
      break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return input;
}

list_t* promptMultiple(const char* text, const char* label, list_t* init,
                       unsigned char cliVerbose) {
  list_t* out = NULL;
  switch (prompt_mode()) {
    case PROMPT_MODE_CLI:
      out = _promptMultipleCLI(cliVerbose ? text : label, init);
      break;
    case PROMPT_MODE_GUI:
      out = _promptMultipleGUI(text, label, init, PROMPT_NO_TIMEOUT);
      break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return out;
}

int _promptConsentGUIDefaultYes(const char* text, const int timeout) {
  char* cmd = oidcPromptCmd("confirm-default-yes", "oidc-agent prompt confirm",
                            text, "", "", timeout);
  char* out = getOutputFromCommand(cmd);
  secFree(cmd);
  int ret = out != NULL && strcaseequal(out, "yes") ? 1 : 0;
  secFree(out);
  return ret;
}

int _promptConsentGUIDefaultNo(const char* text, const int timeout) {
  char* cmd = oidcPromptCmd("confirm-default-no", "oidc-agent prompt confirm",
                            text, "", "", timeout);
  char* out = getOutputFromCommand(cmd);
  secFree(cmd);
  int ret = out != NULL && strcaseequal(out, "yes") ? 1 : 0;
  secFree(out);
  return ret;
}

int promptConsentDefaultNo(const char* text) {
  if (prompt_mode() == PROMPT_MODE_GUI) {
    return _promptConsentGUIDefaultNo(text, 0);
  }
  char* res = prompt(text, NULL, "No/yes/quit", CLI_PROMPT_VERBOSE);
  if (strequal(res, "yes")) {
    secFree(res);
    return 1;
  } else if (strequal(res, "quit")) {
    exit(EXIT_SUCCESS);
  } else {
    secFree(res);
    return 0;
  }
}

int promptConsentDefaultYes(const char* text) {
  if (prompt_mode() == PROMPT_MODE_GUI) {
    return _promptConsentGUIDefaultNo(text, 0);
  }
  char* res = prompt(text, NULL, "Yes/no/quit", CLI_PROMPT_VERBOSE);
  if (strequal(res, "no")) {
    secFree(res);
    return 0;
  } else if (strequal(res, "quit")) {
    exit(EXIT_SUCCESS);
  } else {
    secFree(res);
    return 1;
  }
}
