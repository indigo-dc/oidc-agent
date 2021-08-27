#ifndef OIDC_AGENT_STRINGBUILDER_H
#define OIDC_AGENT_STRINGBUILDER_H

#include <stddef.h>

struct str_builder;
typedef struct str_builder str_builder_t;

/**
 * @brief Create a str builder.
 * @return str builder.
 */
str_builder_t* str_builder_create(size_t alloc_interval);

/**
 * @brief Frees a str builder.
 * @param sb Builder.
 */
void secFree_str_builder(str_builder_t* sb);

/**
 * @brief Add a string to the builder.
 * @param sb  Builder.
 * @param str String to add.
 */
void str_builder_add_str(str_builder_t* sb, const char* str);

/**
 * @brief Add a character to the builder.
 * @param sb Builder.
 * @param c  Character.
 */
void str_builder_add_char(str_builder_t* sb, char c);

/**
 * @brief Add an integer as to the builder.
 * @param sb  Builder.
 * @param val Int to add.
 */
void str_builder_add_int(str_builder_t* sb, int val);

/**
 * @brief Returns the length of the string contained in the builder.
 * @param sb Builder.
 * @return Length.
 */
size_t str_builder_len(const str_builder_t* sb);

/**
 * @brief Returns a copy of the string data.
 * @param  sb  Builder.
 * @return Copy of the internal string data, must be freed using @c secFree.
 */
char* str_builder_get_string(const str_builder_t* sb);

#endif  // OIDC_AGENT_STRINGBUILDER_H
