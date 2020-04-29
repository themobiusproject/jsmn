/*
 * MIT License
 *
 * Copyright (c) 2010 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __JSMNM_H_
#define __JSMNM_H_

#ifdef UNIT_TESTING
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#endif

#include <stddef.h>

#include "jsmn_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef JSMN_STATIC
#define JSMN_API static
#else
#define JSMN_API extern
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
  JSMN_PRIMITIVE = 0x08,
} jsmntype_t;

enum jsmnerr {
  /* Not enough tokens were provided */
  JSMN_ERROR_NOMEM = -1,
  /* Invalid character inside JSON string */
  JSMN_ERROR_INVAL = -2,
  /* The string is not a full JSON packet, more bytes expected */
  JSMN_ERROR_PART  = -3,
  /* Input data too long */
  JSMN_ERROR_LEN   = -4,
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
 * type   type (object, array, string etc.)
 * start  start position in JSON data string
 * end    end position in JSON data string
 */
typedef struct jsmntok_t {
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
 * the string being parsed now and current position in that string.
 */
typedef struct jsmn_parser {
  jsmnint_t pos;        /* offset in the JSON string */
  jsmnint_t toknext;    /* next token to allocate */
  jsmnint_t toksuper;   /* superior token node, e.g. parent object or array */
} jsmn_parser;

/**
 * @brief Create JSON parser over an array of tokens
 *
 * @param[out] parser jsmn parser
 */
JSMN_API void jsmn_init(jsmn_parser *parser);

/**
 * @brief Run JSON parser. It parses a JSON data string into and array of
 * tokens, each describing a single JSON object.
 *
 * @param[in,out] parser jsmn parser
 * @param[in] js JSON data string
 * @param[in] len JSON data string length
 * @param[in,out] tokens pointer to memory allocated for tokens or NULL
 * @param[in] num_tokens number of tokens allocated
 * @return jsmnint_t number of tokens found or ERRNO
 */
JSMN_API jsmnint_t jsmn_parse(jsmn_parser *parser, const char *js,
                              const size_t len, jsmntok_t *tokens,
                              const unsigned int num_tokens);

#ifdef __cplusplus
}
#endif

#endif /* __JSMNM_H_ */
