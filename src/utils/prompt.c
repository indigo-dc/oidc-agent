#define _XOPEN_SOURCE 700

#include "prompt.h"
#include "memory.h"
#include "oidc_error.h"
#include "printer.h"
#include "utils/file_io/file_io.h"
#include "utils/listUtils.h"
#include "utils/logger.h"
#include "utils/prompt_mode.h"
#include "utils/stringUtils.h"
#include "utils/system_runner.h"

#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

char* _promptPasswordGUI(const char* text, const char* label,
                         const char* init) {
  char* cmd = oidc_sprintf("oidc-prompt password \"oidc-agent prompt\" \"%s\" "
                           "\"%s\" \"%s\"",
                           text, label, init ?: "");
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret;
}

char* _promptGUI(const char* text, const char* label, const char* init) {
  char* cmd = oidc_sprintf(
      "oidc-prompt input \"oidc-agent prompt\" \"%s\" \"%s\" \"%s\"", text,
      label, init ?: "");
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  return ret;
}

char* _promptSelectGUI(const char* text, const char* label, list_t* init,
                       size_t initPos) {
  list_t* copy = list_new();
  copy->free   = _secFree;
  for (size_t i = 0; i < init->len; i++) {
    if (i != initPos) {
      list_rpush(copy,
                 list_node_new(oidc_sprintf("\"%s\"", list_at(init, i)->val)));
    }
  }
  char* options = listToDelimitedString(copy, " ");
  secFreeList(copy);
  char* cmd = oidc_sprintf(
      "oidc-prompt select-other \"oidc-agent prompt\" \"%s\" \"%s\" \"%s\" %s",
      text, label, list_at(init, initPos)->val,options);
  secFree(options);
  char* ret = getOutputFromCommand(cmd);
  secFree(cmd);
  if (!strValid(ret)) {  // Cancel
    raise(SIGINT);       // Cancel should have the same behaviour as interrupt
  }
  return ret;
}

list_t* _promptMultipleGUI(const char* text, const char* label, list_t* init) {
  char* inits  = listToDelimitedString(init, " ");
  char* text_p = oidc_sprintf("%s (One value per line)", text);
  char* cmd    = oidc_sprintf(
      "oidc-prompt multiple \"oidc-agent prompt\" \"%s\" \"%s\" %s", text_p,
      label, inits);
  secFree(text_p);
  secFree(inits);
  char* input = getOutputFromCommand(cmd);
  secFree(cmd);
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
    printPrompt("[%lu] %s\n", i + 1, (char*)list_at(suggastable, i)->val);
  }
}

char* _promptSelectCLI(const char* text, list_t* init, size_t initPos) {
  char* fav = NULL;
  if (init == NULL || list_at(init, initPos) == NULL) {
    logger(ERROR, "In %s initPos not valid", __func__);
  } else {
    fav = list_at(init, initPos)->val;
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
      out = oidc_strcopy(list_at(init, i)->val);
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

/** @fn char* promptPassword(char* prompt_str, ...)
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
      password = _promptPasswordGUI(text, label, init);
      break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return password;
}

/** @fn char* prompt(char* prompt_str, ...)
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
    case PROMPT_MODE_GUI: input = _promptGUI(text, label, init); break;
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
      input = _promptSelectGUI(text, label, options, initPos);
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
    case PROMPT_MODE_GUI: out = _promptMultipleGUI(text, label, init); break;
    default:
      logger(ERROR, "Invalid prompt mode");
      oidc_setInternalError("Programming error: Prompt Mode must be set!");
      oidc_perror();
      return NULL;
  }
  return out;
}

int _promptConsentGUIDefaultYes(const char* text) {
  char* cmd = oidc_sprintf("oidc-prompt confirm-default-yes "
                           "\"oidc-agent prompt confirm\" \"%s\"",
                           text);
  char* out = getOutputFromCommand(cmd);
  secFree(cmd);
  int ret = out != NULL && strcaseequal(out, "yes") ? 1 : 0;
  secFree(out);
  return ret;
}

int _promptConsentGUIDefaultNo(const char* text) {
  char* cmd = oidc_sprintf(
      "oidc-prompt confirm-default-no \"oidc-agent prompt confirm\" \"%s\"",
      text);
  char* out = getOutputFromCommand(cmd);
  secFree(cmd);
  int ret = out != NULL && strcaseequal(out, "yes") ? 1 : 0;
  secFree(out);
  return ret;
}

int promptConsentDefaultNo(const char* text) {
  if (prompt_mode() == PROMPT_MODE_GUI) {
    return _promptConsentGUIDefaultNo(text);
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
    return _promptConsentGUIDefaultNo(text);
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
