#ifndef OIDC_AGENT_STRINGBUILDER_H
#define OIDC_AGENT_STRINGBUILDER_H

#include <stddef.h>

struct str_builder;
typedef struct str_builder str_builder_t;

/*! Create a str builder.
 *
 * return str builder.
 */
str_builder_t* str_builder_create(size_t alloc_interval);

/*! Destroy a str builder.
 *
 * param[in,out] sb Builder.
 */
void secFree_str_builder(str_builder_t* sb);

/*! Add a string to the builder.
 *
 * param[in,out] sb  Builder.
 * param[in]     str String to add.
 */
void str_builder_add_str(str_builder_t* sb, const char* str);

/*! Add a character to the builder.
 *
 * param[in,out] sb Builder.
 * param[in]     c  Character.
 */
void str_builder_add_char(str_builder_t* sb, char c);

/*! Add an integer as to the builder.
 *
 * param[in,out] sb  Builder.
 * param[in]     val Int to add.
 */
void str_builder_add_int(str_builder_t* sb, int val);

/*! The length of the string contained in the builder.
 *
 * param[in] sb Builder.
 *
 * return Length.
 */
size_t str_builder_len(const str_builder_t* sb);

/*! Return a copy of the string data.
 *
 * param[in]  sb  Builder.
 *
 * return Copy of the internal string data.
 */
char* str_builder_get_string(const str_builder_t* sb);

#endif  // OIDC_AGENT_STRINGBUILDER_H
