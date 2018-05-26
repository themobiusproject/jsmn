#ifndef __JSON_H__
#define __JSON_H__

#include <stdarg.h>
#include <stdint.h>

#include "jsmn.h"

#ifdef __cplusplus
extern "C" {
#endif

char *readJSONFile(const char *filename);

/**
 * @brief Tokenizes JSON string
 *
 * @param[in] json JSON String
 * @param[out] rv Return Value
 * @return Allocated jsmntok_t array pointer
 */
jsmntok_t *json_tokenize(char *json, size_t json_len, jsmnint_t *rv);

/**
 * @brief String comparison between token and string
 *
 * @param[in] json JSON String
 * @param[in] tok Token to compare
 * @param[in] s String to complare
 * @return 0 when token string and s are equal, -1 otherwise
 */
int json_token_streq(const char *json, const jsmntok_t *tok, const char *s);

/**
* @brief Parse a json string and return the value of the key requested
* 
* @param[in] json json string
* @param[in] tokens jsmn tokens
* @param[in] num_keys number of keys
* @return jsmnint_t value
*/
jsmnint_t json_parse(const char *json, const jsmntok_t *tokens, const uint32_t num_keys, ...);

jsmnint_t isJSONKey(const jsmntok_t *tokens, const jsmnint_t t);
jsmnint_t isJSONArrayMember(const jsmntok_t *tokens, const jsmnint_t t);
jsmnint_t getJSONKeyValue(const jsmntok_t *tokens, const jsmnint_t t);
jsmnint_t jsmnTokenLen(const jsmntok_t *tok);

#ifdef __cplusplus
}
#endif

#endif // __JSON_H__
