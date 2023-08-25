/*
 Author: José Bollo <jobol@nonadev.net>

 https://gitlab.com/jobol/mustach

 SPDX-License-Identifier: ISC
*/

#ifndef _mustach_cJSON_h_included_
#define _mustach_cJSON_h_included_

/*
 * mustach-json-c is intended to make integration of cJSON
 * library by providing integrated functions.
 */

#include "mustach-wrap.h"
#include "wrapper/cjson.h"

/**
 * Wrap interface used internally by mustach cJSON functions.
 * Can be used for overriding behaviour.
 */
extern const struct mustach_wrap_itf mustach_cJSON_wrap_itf;

/**
 * mustach_cJSON_file - Renders the mustache 'template' in 'file' for 'root'.
 *
 * @template: the template string to instanciate
 * @length:   length of the template or zero if unknown and template null
 * terminated
 * @root:     the root json object to render
 * @file:     the file where to write the result
 *
 * Returns 0 in case of success, -1 with errno set in case of system error
 * a other negative value in case of error.
 */
extern int mustach_cJSON_file(const char* template, size_t length, cJSON* root,
                              int flags, FILE* file);

/**
 * mustach_cJSON_fd - Renders the mustache 'template' in 'fd' for 'root'.
 *
 * @template: the template string to instanciate
 * @length:   length of the template or zero if unknown and template null
 * terminated
 * @root:     the root json object to render
 * @fd:       the file descriptor number where to write the result
 *
 * Returns 0 in case of success, -1 with errno set in case of system error
 * a other negative value in case of error.
 */
extern int mustach_cJSON_fd(const char* template, size_t length, cJSON* root,
                            int flags, int fd);

/**
 * mustach_cJSON_mem - Renders the mustache 'template' in 'result' for 'root'.
 *
 * @template: the template string to instanciate
 * @length:   length of the template or zero if unknown and template null
 * terminated
 * @root:     the root json object to render
 * @result:   the pointer receiving the result when 0 is returned
 * @size:     the size of the returned result
 *
 * Returns 0 in case of success, -1 with errno set in case of system error
 * a other negative value in case of error.
 */
extern int mustach_cJSON_mem(const char* template, size_t length, cJSON* root,
                             int flags, char** result, size_t* size);

/**
 * mustach_cJSON_write - Renders the mustache 'template' for 'root' to custom
 * writer 'writecb' with 'closure'.
 *
 * @template: the template string to instanciate
 * @length:   length of the template or zero if unknown and template null
 * terminated
 * @root:     the root json object to render
 * @writecb:  the function that write values
 * @closure:  the closure for the write function
 *
 * Returns 0 in case of success, -1 with errno set in case of system error
 * a other negative value in case of error.
 */
extern int mustach_cJSON_write(const char* template, size_t length, cJSON* root,
                               int flags, mustach_write_cb_t* writecb,
                               void* closure);

/**
 * mustach_cJSON_emit - Renders the mustache 'template' for 'root' to custom
 * emiter 'emitcb' with 'closure'.
 *
 * @template: the template string to instanciate
 * @length:   length of the template or zero if unknown and template null
 * terminated
 * @root:     the root json object to render
 * @emitcb:   the function that emit values
 * @closure:  the closure for the write function
 *
 * Returns 0 in case of success, -1 with errno set in case of system error
 * a other negative value in case of error.
 */
extern int mustach_cJSON_emit(const char* template, size_t length, cJSON* root,
                              int flags, mustach_emit_cb_t* emitcb,
                              void* closure);

#endif
