#ifndef __JSON_H__
#define __JSON_H__

#include <stdarg.h>
#include <stdint.h>

#ifndef JSMNM_DEFINES
#define JSMNM_DEFINES
#define JSMNM_STRICT
#define JSMNM_PARENT_LINKS
#define JSMNM_SHORT_TOKENS
#define JSMNM_NEXT_SIBLING
#endif

#ifdef ARDUINO
#define NDEBUG
#endif

#include "jsmnm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return a pointer to jsmnm error message
 *
 * @param[in] errno jsmnm error number
 * @return const char* jsmnm error message
 */
const char *jsmnm_strerror(jsmnmenumtype_t errno);

/**
 * @brief Tokenizes JSON string
 *
 * @param[in] json JSON String
 * @param[out] rv Return Value
 * @return Allocated jsmnmtok_t array pointer
 */
jsmnmtok_t *json_tokenize(const char *json, size_t json_len, jsmnmint_t *rv);

/**
 * @brief Parse a json string and return the value of the key requested
 *
 * @param[in] json json string
 * @param[in] tokens jsmnm tokens
 * @param[in] num_keys number of keys
 * @return jsmnmint_t value
 */
jsmnmint_t json_parse(const char *json, const jsmnmtok_t *tokens, const uint32_t num_keys, ...);

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
