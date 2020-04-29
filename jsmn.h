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
#ifndef JSMN_H
#define JSMN_H

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

#ifndef JSMN_HEADER
/**
 * Allocates a fresh unused token from the token pool.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser, jsmntok_t *tokens,
                                   const size_t num_tokens) {
  jsmntok_t *tok;
  if (parser->toknext >= num_tokens) {
    return NULL;
  }
  tok = &tokens[parser->toknext++];
  tok->start = tok->end = JSMN_NEG;
  tok->size = 0;
#ifdef JSMN_PARENT_LINKS
  tok->parent = JSMN_NEG;
#endif
#ifdef JSMN_NEXT_SIBLING
  tok->next_sibling = JSMN_NEG;
#endif
  return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, const jsmnenumtype_t type,
                            const jsmnint_t start, const jsmnint_t end) {
  token->type = type;
  token->start = start;
  token->end = end;
  token->size = 0;
}

#ifdef JSMN_NEXT_SIBLING
/**
 * Set previous child's next_sibling to current token
 */
static void jsmn_next_sibling(jsmn_parser *parser, jsmntok_t *tokens) {
  /* Ensure current token has a parent */
  if (parser->toksuper == JSMN_NEG)
    return;

  /* Start with parent's first child */
  jsmnint_t sibling = parser->toksuper + 1;

  /* If the first child is the current token */
  if (sibling == parser->toknext - 1)
    return;

  /* Loop until we find previous sibling */
  while (tokens[sibling].next_sibling != JSMN_NEG)
    sibling = tokens[sibling].next_sibling;

  /* Set previous sibling's next_sibling to current token */
  tokens[sibling].next_sibling = parser->toknext - 1;
}
#endif

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js,
                                const size_t len, jsmntok_t *tokens,
                                const size_t num_tokens) {
  jsmntok_t *token;
  jsmnint_t start;

  start = parser->pos;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
#ifdef JSMN_DEBUG
    printf(" -> Primitive");
#endif
    switch (js[parser->pos]) {
#ifndef JSMN_STRICT
    /* In strict mode primitive must be followed by "," or "}" or "]" */
    case ':':
#endif
    case '\t':
    case '\r':
    case '\n':
    case ' ':
    case ',':
    case ']':
    case '}':
      goto found;
    default:
                   /* to quiet a warning from gcc*/
      break;
    }
    if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
      parser->pos = start;
#ifdef JSMN_DEBUG
      printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
      return JSMN_ERROR_INVAL;
    }
  }
#ifdef JSMN_STRICT
  /* In strict mode primitive must be followed by a comma/object/array */
  parser->pos = start;
#ifdef JSMN_DEBUG
  printf(" -> JSMN_ERROR_PART %d\n", __LINE__);
#endif
  return JSMN_ERROR_PART;
#endif

found:
  if (tokens == NULL) {
    parser->pos--;
    return 0;
  }
  token = jsmn_alloc_token(parser, tokens, num_tokens);
  if (token == NULL) {
    parser->pos = start;
#ifdef JSMN_DEBUG
    printf(" -> JSMN_ERROR_NOMEM %d\n", __LINE__);
#endif
    return JSMN_ERROR_NOMEM;
  }
  jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
  token->parent = parser->toksuper;
#endif
#ifdef JSMN_NEXT_SIBLING
  jsmn_next_sibling(parser, tokens);
#endif
  parser->pos--;
  return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js,
                             const size_t len, jsmntok_t *tokens,
                             const size_t num_tokens) {
  if (len >= JSMN_NEG) {
#ifdef JSMN_DEBUG
    printf(" -> JSMN_ERROR_LEN %d\n", __LINE__);
#endif
    return JSMN_ERROR_LEN;
  }

  char c;
  int i;
  jsmntok_t *token;

  jsmnint_t start = parser->pos;

  parser->pos++;

  /* Skip starting quote */
  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
#ifdef JSMN_DEBUG
    printf(" -> String");
#endif
    c = js[parser->pos];
#ifdef JSMN_DEBUG
    printf("\nJSON Position: %05d  Character: %c", parser->pos, c);
#endif

    /* Quote: end of string */
    if (c == '\"') {
#ifdef JSMN_DEBUG
      printf(" -> \"");
#endif
      if (tokens == NULL) {
        return 0;
      }
      token = jsmn_alloc_token(parser, tokens, num_tokens);
      if (token == NULL) {
        parser->pos = start;
#ifdef JSMN_DEBUG
        printf(" -> JSMN_ERROR_NOMEM %d\n", __LINE__);
#endif
        return JSMN_ERROR_NOMEM;
      }
      jsmn_fill_token(token, JSMN_STRING, start + 1, parser->pos);
#ifdef JSMN_PARENT_LINKS
      token->parent = parser->toksuper;
#endif
#ifdef JSMN_NEXT_SIBLING
      jsmn_next_sibling(parser, tokens);
#endif
      return 0;
    }

    /* Backslash: Quoted symbol expected */
    if (c == '\\' && parser->pos + 1 < len) {
#ifdef JSMN_DEBUG
      printf(" -> \\\n");
#endif
      parser->pos++;
      switch (js[parser->pos]) {
      /* Allowed escaped symbols */
      case '\"':
      case '/':
      case '\\':
      case 'b':
      case 'f':
      case 'r':
      case 'n':
      case 't':
        break;
      /* Allows escaped symbol \uXXXX */
      case 'u':
        parser->pos++;
        for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++) {
          /* If it isn't a hex character we have an error */
          if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) ||   /* 0-9 */
                (js[parser->pos] >= 65 && js[parser->pos] <= 70) ||   /* A-F */
                (js[parser->pos] >= 97 && js[parser->pos] <= 102))) { /* a-f */
            parser->pos = start;
#ifdef JSMN_DEBUG
            printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
            return JSMN_ERROR_INVAL;
          }
          parser->pos++;
        }
        parser->pos--;
        break;
      /* Unexpected symbol */
      default:
        parser->pos = start;
#ifdef JSMN_DEBUG
        printf(" -> JSMN_ERROR_PART %d\n", __LINE__);
#endif
        return JSMN_ERROR_INVAL;
      }
    }
  }
  parser->pos = start;
#ifdef JSMN_DEBUG
  printf(" -> JSMN_ERROR_PART %d\n", __LINE__);
#endif
  return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
JSMN_API jsmnint_t jsmn_parse(jsmn_parser *parser, const char *js,
                              const size_t len, jsmntok_t *tokens,
                              const unsigned int num_tokens) {
  if (len >= JSMN_NEG) {
#ifdef JSMN_DEBUG
    printf(" -> JSMN_ERROR_LEN %d\n", __LINE__);
#endif
    return JSMN_ERROR_LEN;
  }

  int r;
  int i;
  jsmntok_t *token, *t;
  jsmnint_t count = parser->toknext;

  char c;
  jsmnenumtype_t type;
  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
    c = js[parser->pos];
#ifdef JSMN_DEBUG
    printf("\nJSON Position: %05d  Character: %c", parser->pos, c);
#endif
    switch (c) {
    case '{':
    case '[':
#ifdef JSMN_DEBUG
      printf(" -> { or [");
#endif
      count++;
      if (tokens == NULL) {
        break;
      }
      token = jsmn_alloc_token(parser, tokens, num_tokens);
      if (token == NULL) {
#ifdef JSMN_DEBUG
        printf(" -> JSMN_ERROR_NOMEM %d\n", __LINE__);
#endif
        return JSMN_ERROR_NOMEM;
      }
      if (parser->toksuper != JSMN_NEG) {
        t = &tokens[parser->toksuper];
//#ifdef JSMN_STRICT
//        /* In strict mode an object or array can't become a key */
//        if (t->type == JSMN_OBJECT) {
#ifdef JSMN_DEBUG
          printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
//          return JSMN_ERROR_INVAL;
//        }
//#endif
        t->size++;
#ifdef JSMN_PARENT_LINKS
        token->parent = parser->toksuper;
#endif
#ifdef JSMN_NEXT_SIBLING
        jsmn_next_sibling(parser, tokens);
#endif
      }
      token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
      token->start = parser->pos;
      parser->toksuper = parser->toknext - 1;
      break;
    case '}':
    case ']':
#ifdef JSMN_DEBUG
      printf(" -> } or ]");
#endif
      if (tokens == NULL) {
        break;
      }
      type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
      if (parser->toknext < 1) {
#ifdef JSMN_DEBUG
        printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
        return JSMN_ERROR_INVAL;
      }
      token = &tokens[parser->toknext - 1];
      for (;;) {
        if (token->start != JSMN_NEG && token->end == JSMN_NEG) {
          if (token->type != type) {
#ifdef JSMN_DEBUG
            printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
            return JSMN_ERROR_INVAL;
          }
          token->end = parser->pos + 1;
          parser->toksuper = token->parent;
          break;
        }
        if (token->parent == JSMN_NEG) {
          if (token->type != type || parser->toksuper == JSMN_NEG) {
#ifdef JSMN_DEBUG
            printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
            return JSMN_ERROR_INVAL;
          }
          break;
        }
        token = &tokens[token->parent];
      }
#else
      for (i = parser->toknext - 1; i >= 0; i--) {
        token = &tokens[i];
        if (token->start != JSMN_NEG && token->end == JSMN_NEG) {
          if (token->type != type) {
#ifdef JSMN_DEBUG
            printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
            return JSMN_ERROR_INVAL;
          }
          parser->toksuper = JSMN_NEG;
          token->end = parser->pos + 1;
          break;
        }
      }
      /* Error if unmatched closing bracket */
      if (i == -1) {
#ifdef JSMN_DEBUG
        printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
        return JSMN_ERROR_INVAL;
      }
      for (; i >= 0; i--) {
        token = &tokens[i];
        if (token->start != JSMN_NEG && token->end == JSMN_NEG) {
          parser->toksuper = i;
          break;
        }
      }
#endif
      break;
    case '\"':
#ifdef JSMN_DEBUG
      printf(" -> \"");
#endif
      r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
      if (r < 0) {
        return r;
      }
      count++;
      if (parser->toksuper != JSMN_NEG && tokens != NULL) {
        tokens[parser->toksuper].size++;
      }
      break;
    case '\t':
    case '\r':
    case '\n':
    case ' ':
#ifdef JSMN_DEBUG
      printf(" -> \\t \\r \\n");
#endif
      break;
    case ':':
#ifdef JSMN_DEBUG
      printf(" -> :");
#endif
      parser->toksuper = parser->toknext - 1;
      break;
    case ',':
#ifdef JSMN_DEBUG
      printf(" -> ,");
#endif
      if (tokens != NULL && parser->toksuper != JSMN_NEG &&
          tokens[parser->toksuper].type != JSMN_ARRAY &&
          tokens[parser->toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
        parser->toksuper = tokens[parser->toksuper].parent;
#else
        for (i = parser->toknext - 1; i >= 0; i--) {
          if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT) {
            if (tokens[i].start != JSMN_NEG && tokens[i].end == JSMN_NEG) {
              parser->toksuper = i;
              break;
            }
          }
        }
#endif
      }
      break;
#ifdef JSMN_STRICT
    /* In strict mode primitives are: numbers and booleans */
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 't':
    case 'f':
    case 'n':
#ifdef JSMN_DEBUG
      printf(" -> 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, t, f, n");
#endif
      /* And they must not be keys of the object */
      if (tokens != NULL && parser->toksuper != JSMN_NEG) {
        t = &tokens[parser->toksuper];
        if (t->type == JSMN_OBJECT ||
            (t->type == JSMN_STRING && t->size != 0)) {
#ifdef JSMN_DEBUG
          printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
          return JSMN_ERROR_INVAL;
        }
      }
#else
    /* In non-strict mode every unquoted value is a primitive */
    default:
#endif
      r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
      if (r < 0) {
        return r;
      }
      count++;
      if (parser->toksuper != JSMN_NEG && tokens != NULL) {
        tokens[parser->toksuper].size++;
      }
      break;

#ifdef JSMN_STRICT
    /* Unexpected char in strict mode */
    default:
#ifdef JSMN_DEBUG
      printf(" -> JSMN_ERROR_INVAL %d\n", __LINE__);
#endif
      return JSMN_ERROR_INVAL;
#endif
    }
  }

  if (tokens != NULL) {
    for (i = parser->toknext - 1; i >= 0; i--) {
      /* Unmatched opened object or array */
      if (tokens[i].start != JSMN_NEG && tokens[i].end == JSMN_NEG) {
#ifdef JSMN_DEBUG
        printf(" -> JSMN_ERROR_PART %d\n", __LINE__);
#endif
        return JSMN_ERROR_PART;
      }
    }
  }

#ifdef JSMN_DEBUG
  printf(" -> count %d\n", __LINE__);
#endif
  return count;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
JSMN_API void jsmn_init(jsmn_parser *parser) {
  parser->pos = 0;
  parser->toknext = 0;
  parser->toksuper = JSMN_NEG;
}

#endif /* JSMN_HEADER */

#ifdef __cplusplus
}
#endif

#endif /* JSMN_H */
