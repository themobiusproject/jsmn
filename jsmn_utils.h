#ifndef __JSMN_UTILS_H__
#define __JSMN_UTILS_H__

#include <stdarg.h>
#include <stdint.h>

#include "jsmn_defines.h"

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
const char *jsmn_strerror(enum jsmnerr errno);

/**
 * @brief Tokenizes JSON string
 *
 * @param[in] json JSON String
 * @param[in] json_len Length of JSON String
 * @param[out] rv Return Value
 * @return Allocated jsmntok_t array pointer
 */
jsmntok_t *jsmn_tokenize(const char *json, const size_t json_len, jsmnint_t *rv);


/**
 * @brief Tokenize JSON string
 *
 * @param[out] tokens Pointer to preallocated Tokens
 * @param[in] num_tokens Number of Tokens
 * @param[in] json JSON String
 * @param[in] json_len Length of JSON String
 * @return Return Value
 */
jsmnint_t jsmn_tokenize_noalloc(jsmntok_t *tokens, const uint32_t num_tokens, const char *json, const size_t json_len);

/**
 * @brief Parse a json string and return the value of the key requested
 *
 * @param[in] json json string
 * @param[in] tokens jsmn tokens
 * @param[in] num_keys number of keys
 * @return jsmnint_t value
 */
jsmnint_t jsmn_parse(const char *json, const jsmntok_t *tokens, const size_t num_keys, ...);

/**
 * @brief Print an extremely verbose description of JSON string
 *
 * @param[in] json JSON String
 * @param[in] len Length of JSON String
 */
void explodeJSON(const char *json, size_t len);

#ifdef __cplusplus
}
#endif

#endif // __JSMN_UTILS_H__
