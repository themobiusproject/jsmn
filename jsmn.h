#ifndef __JSMN_H_
#define __JSMN_H_

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
typedef enum {
	JSMN_UNDEFINED = 0x00,
	JSMN_OBJECT    = 0x01,
	JSMN_ARRAY     = 0x02,
	JSMN_STRING    = 0x04,
	JSMN_PRIMITIVE = 0x08
} jsmntype_t;

enum jsmnerr {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3,
	/* Input data too long */
	JSMN_ERROR_LEN = -4
};

#ifdef JSMN_SHORT_TOKENS
typedef unsigned short jsmnint_t;
typedef char jsmnenumtype_t;
#else
typedef unsigned int jsmnint_t;
typedef jsmntype_t jsmnenumtype_t;
#endif
#define JSMN_NEG ((jsmnint_t)-1)

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
	jsmnenumtype_t type;
	jsmnint_t start;
	jsmnint_t end;
	jsmnint_t size;
#ifdef JSMN_PARENT_LINKS
	jsmnint_t parent;
#endif
#ifdef JSMN_NEXT_SIBLING
	jsmnint_t next_sibling;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
	jsmnint_t pos; /* offset in the JSON string */
	jsmnint_t toknext; /* next token to allocate */
	jsmnint_t toksuper; /* superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
jsmnint_t jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */
