#ifndef __JSON_H__
#define __JSON_H__

#include <stdarg.h>
#include <stdint.h>

#define JSMN_STRICT
#define JSMN_SHORT_TOKENS
#define JSMN_NEXT_SIBLING
#define JSMN_PARENT_LINKS

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
jsmntok_t *json_tokenize(char *json, size_t json_len, jsmnint_t *rv);

/**
* @brief Parse a json string and return the value of the key requested
* 
* @param[in] json json string
* @param[in] tokens jsmn tokens
* @param[in] num_keys number of keys
* @return jsmnint_t value
*/
jsmnint_t json_parse(const char *json, const jsmntok_t *tokens, const uint32_t num_keys, ...);

#ifdef __cplusplus
}
#endif

#endif // __JSON_H__
