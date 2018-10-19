#ifndef __JSMNM_H_
#define __JSMNM_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum jsmnmtype_t {
	JSMNM_UNDEFINED = 0x00,
	JSMNM_OBJECT    = 0x01,
	JSMNM_ARRAY     = 0x02,
	JSMNM_STRING    = 0x04,
	JSMNM_PRIMITIVE = 0x08,
} jsmnmtype_t;

enum jsmnmerr {
	/* Not enough tokens were provided */
	JSMNM_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMNM_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMNM_ERROR_PART  = -3,
	/* Input data too long */
	JSMNM_ERROR_LEN   = -4,
};

#ifdef JSMNM_SHORT_TOKENS
typedef unsigned short jsmnmint_t;
typedef char jsmnmenumtype_t;
#else
typedef unsigned int jsmnmint_t;
typedef jsmnmtype_t jsmnmenumtype_t;
#endif
#define JSMN_NEG ((jsmnmint_t)-1)

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct jsmnmtok_t {
	jsmnmenumtype_t type;
	jsmnmint_t start;
	jsmnmint_t end;
	jsmnmint_t size;
#ifdef JSMNM_PARENT_LINKS
	jsmnmint_t parent;
#endif
#ifdef JSMNM_NEXT_SIBLING
	jsmnmint_t next_sibling;
#endif
} jsmnmtok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct jsmnm_parser {
	jsmnmint_t pos; /* offset in the JSON string */
	jsmnmint_t toknext; /* next token to allocate */
	jsmnmint_t toksuper; /* superior token node, e.g parent object or array */
} jsmnm_parser;

/**
 * @brief Create JSON parser over an array of tokens
 *
 * @param[out] parser jsmnm parser
 */
void jsmnm_init(jsmnm_parser *parser);

/**
 * @brief Run JSON parser. It parses a JSON data string into and array of
 * tokens, each describing a single JSON object.
 *
 * @param[in,out] parser jsmnm parser
 * @param[in] js JSON data string
 * @param[in] len JSON data string length
 * @param[in,out] tokens pointer to memory allocated for tokens or NULL
 * @param[in] num_tokens number of tokens allocated
 * @return jsmnmint_t number of tokens found or ERRNO
 */
jsmnmint_t jsmnm_parse(jsmnm_parser *parser, const char *js, size_t len,
		jsmnmtok_t *tokens, unsigned int num_tokens);

#ifdef __cplusplus
}
#endif

#endif /* __JSMNM_H_ */
