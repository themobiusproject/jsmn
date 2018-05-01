#ifndef __JSON_H__
#define __JSON_H__

#include <stdarg.h>
#include <stdint.h>

#include "jsmn.h"

__BEGIN_DECLS

char *readJSONFile(const char *filename);

/**
 * @brief Tokenizes JSON string
 *
 * @param json JSON String
 * @param rv Return Value
 * @return Allocated jsmntok_t array pointer
 */
jsmntok_t *json_tokenize(char *json, size_t json_len, jsmnint_t *rv);

/**
 * @brief String comparison between token and string
 *
 * @param json JSON String
 * @param tok Token to compare
 * @param s String to complare
 * @return 0 when token string and s are equal, -1 otherwise
 */
int json_token_streq(const char *json, jsmntok_t *tok, const char *s);

/**
* @brief Parse a json string and return the value of the key requested
* 
* @param json json string
* @param tokens jsmn tokens
* @param num_keys number of keys
* @return jsmnint_t value
*/
jsmnint_t json_parse(const char *json, jsmntok_t *tokens, uint32_t num_keys, ...);

jsmnint_t isJSONKey(jsmntok_t *tokens, jsmnint_t t);
jsmnint_t isJSONArrayMember(jsmntok_t *tokens, jsmnint_t t);
jsmnint_t getJSONKeyValue(jsmntok_t *tokens, jsmnint_t t);
jsmnint_t jsmnTokenLen(jsmntok_t *tok);

__END_DECLS

#endif // __JSON_H__
