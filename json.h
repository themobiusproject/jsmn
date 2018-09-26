#ifndef __JSON_H__
#define __JSON_H__

#include <stdarg.h>
#include <stdint.h>

#ifndef JSMN_DEFINES
#define JSMN_DEFINES
#define JSMN_STRICT
#define JSMN_PARENT_LINKS
#define JSMN_SHORT_TOKENS
#define JSMN_NEXT_SIBLING
#endif

#include "jsmn.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return a pointer to jsmn error message
 *
 * @param[in] errno jsmn error number
 * @return const char* jsmn error message
 */
const char *jsmn_strerror(jsmnenumtype_t errno);

/**
 * @brief Tokenizes JSON string
 *
 * @param[in] json JSON String
 * @param[out] rv Return Value
 * @return Allocated jsmntok_t array pointer
 */
jsmntok_t *json_tokenize(const char *json, size_t json_len, jsmnint_t *rv);

/**
 * @brief Parse a json string and return the value of the key requested
 *
 * @param[in] json json string
 * @param[in] tokens jsmn tokens
 * @param[in] num_keys number of keys
 * @return jsmnint_t value
 */
jsmnint_t json_parse(const char *json, const jsmntok_t *tokens, const uint32_t num_keys, ...);

/**
 * @brief Print an extremely verbose description of JSON string
 *
 * @param[in] json JSON String
 * @param[in] len Length of JSON String
 */
void explodeJSON(const char *json, size_t len);
/**
 * @brief Print an extremely verbose description of JSON string
 *
 * @param[in] json JSON String with escape character '\0' at the end
 */
void explodeJSON_nolen(const char *json);

#ifdef __cplusplus
}
#endif

#endif // __JSON_H__
