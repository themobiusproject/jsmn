#include "jsmnm.h"

#ifdef UNIT_TESTING
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#endif

#ifdef JSMNM_DEBUG
#include <stdio.h>
#endif

#ifndef JSMNM_HEADER
/**
 * Allocates a fresh unused token from the token pool.
 */
static jsmnmtok_t *jsmnm_alloc_token(jsmnm_parser *parser, jsmnmtok_t *tokens,
																		 const size_t num_tokens) {
	jsmnmtok_t *tok;
	if (parser->toknext >= num_tokens) {
		return NULL;
	}
	tok = &tokens[parser->toknext++];
	tok->start = tok->end = JSMN_NEG;
	tok->size = 0;
#ifdef JSMNM_PARENT_LINKS
	tok->parent = JSMN_NEG;
#endif
#ifdef JSMNM_NEXT_SIBLING
	tok->next_sibling = JSMN_NEG;
#endif
	return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmnm_fill_token(jsmnmtok_t *token, const jsmnmenumtype_t type,
														const jsmnmint_t start, const jsmnmint_t end) {
	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
}

#ifdef JSMNM_NEXT_SIBLING
/**
 * Set previous child's next_sibling to current token
 */
static void jsmnm_next_sibling(jsmnm_parser *parser, jsmnmtok_t *tokens) {
	/* Ensure current token has a parent */
	if (parser->toksuper == JSMN_NEG)
		return;

	/* Start with parent's first child */
	jsmnmint_t sibling = parser->toksuper + 1;

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
static int jsmnm_parse_primitive(jsmnm_parser *parser, const char *js,
																 const size_t len, jsmnmtok_t *tokens,
																 const size_t num_tokens) {
	jsmnmtok_t *token;
	jsmnmint_t start;

	start = parser->pos;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
#ifdef JSMNM_DEBUG
		printf(" -> Primitive");
#endif
		switch (js[parser->pos]) {
#ifndef JSMNM_STRICT
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
#ifdef JSMNM_DEBUG
			printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
			return JSMNM_ERROR_INVAL;
		}
	}
#ifdef JSMNM_STRICT
	/* In strict mode primitive must be followed by a comma/object/array */
	parser->pos = start;
#ifdef JSMNM_DEBUG
	printf(" -> JSMNM_ERROR_PART %d\n", __LINE__);
#endif
	return JSMNM_ERROR_PART;
#endif

found:
	if (tokens == NULL) {
		parser->pos--;
		return 0;
	}
	token = jsmnm_alloc_token(parser, tokens, num_tokens);
	if (token == NULL) {
		parser->pos = start;
#ifdef JSMNM_DEBUG
		printf(" -> JSMNM_ERROR_NOMEM %d\n", __LINE__);
#endif
		return JSMNM_ERROR_NOMEM;
	}
	jsmnm_fill_token(token, JSMNM_PRIMITIVE, start, parser->pos);
#ifdef JSMNM_PARENT_LINKS
	token->parent = parser->toksuper;
#endif
#ifdef JSMNM_NEXT_SIBLING
	jsmnm_next_sibling(parser, tokens);
#endif
	parser->pos--;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmnm_parse_string(jsmnm_parser *parser, const char *js,
														 const size_t len, jsmnmtok_t *tokens,
														 const size_t num_tokens) {
	if (len >= JSMN_NEG) {
#ifdef JSMNM_DEBUG
		printf(" -> JSMNM_ERROR_LEN %d\n", __LINE__);
#endif
		return JSMNM_ERROR_LEN;
	}

	char c;
	int i;
	jsmnmtok_t *token;

	jsmnmint_t start = parser->pos;

	parser->pos++;

	/* Skip starting quote */
	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
#ifdef JSMNM_DEBUG
		printf(" -> String");
#endif
		c = js[parser->pos];
#ifdef JSMNM_DEBUG
		printf("\nJSON Position: %05d  Character: %c", parser->pos, c);
#endif

		/* Quote: end of string */
		if (c == '\"') {
#ifdef JSMNM_DEBUG
			printf(" -> \"");
#endif
			if (tokens == NULL) {
				return 0;
			}
			token = jsmnm_alloc_token(parser, tokens, num_tokens);
			if (token == NULL) {
				parser->pos = start;
#ifdef JSMNM_DEBUG
				printf(" -> JSMNM_ERROR_NOMEM %d\n", __LINE__);
#endif
				return JSMNM_ERROR_NOMEM;
			}
			jsmnm_fill_token(token, JSMNM_STRING, start+1, parser->pos);
#ifdef JSMNM_PARENT_LINKS
			token->parent = parser->toksuper;
#endif
#ifdef JSMNM_NEXT_SIBLING
			jsmnm_next_sibling(parser, tokens);
#endif
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && parser->pos + 1 < len) {
#ifdef JSMNM_DEBUG
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
#ifdef JSMNM_DEBUG
							printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
							return JSMNM_ERROR_INVAL;
						}
						parser->pos++;
					}
					parser->pos--;
					break;
				/* Unexpected symbol */
				default:
					parser->pos = start;
#ifdef JSMNM_DEBUG
					printf(" -> JSMNM_ERROR_PART %d\n", __LINE__);
#endif
					return JSMNM_ERROR_INVAL;
			}
		}
	}
	parser->pos = start;
#ifdef JSMNM_DEBUG
	printf(" -> JSMNM_ERROR_PART %d\n", __LINE__);
#endif
	return JSMNM_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
JSMNM_API jsmnmint_t jsmnm_parse(jsmnm_parser *parser, const char *js,
															const size_t len, jsmnmtok_t *tokens,
															const unsigned int num_tokens) {
	if (len >= JSMN_NEG) {
#ifdef JSMNM_DEBUG
		printf(" -> JSMNM_ERROR_LEN %d\n", __LINE__);
#endif
		return JSMNM_ERROR_LEN;
	}

	int r;
	int i;
	jsmnmtok_t *token, *t;
	jsmnmint_t count = parser->toknext;

	char c;
	jsmnmenumtype_t type;
	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		c = js[parser->pos];
#ifdef JSMNM_DEBUG
		printf("\nJSON Position: %05d  Character: %c", parser->pos, c);
#endif
		switch (c) {
			case '{':
			case '[':
#ifdef JSMNM_DEBUG
				printf(" -> { or [");
#endif
				count++;
				if (tokens == NULL) {
					break;
				}
				token = jsmnm_alloc_token(parser, tokens, num_tokens);
				if (token == NULL) {
#ifdef JSMNM_DEBUG
					printf(" -> JSMNM_ERROR_NOMEM %d\n", __LINE__);
#endif
					return JSMNM_ERROR_NOMEM;
				}
				if (parser->toksuper != JSMN_NEG) {
					t = &tokens[parser->toksuper];
#ifdef JSMNM_STRICT
					/* In strict mode an object or array can't become a key */
					if (t->type == JSMNM_OBJECT) {
#ifdef JSMNM_DEBUG
						printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
						return JSMNM_ERROR_INVAL;
					}
#endif
					t->size++;
#ifdef JSMNM_PARENT_LINKS
					token->parent = parser->toksuper;
#endif
#ifdef JSMNM_NEXT_SIBLING
					jsmnm_next_sibling(parser, tokens);
#endif
				}
				token->type = (c == '{' ? JSMNM_OBJECT : JSMNM_ARRAY);
				token->start = parser->pos;
				parser->toksuper = parser->toknext - 1;
				break;
			case '}':
			case ']':
#ifdef JSMNM_DEBUG
				printf(" -> } or ]");
#endif
				if (tokens == NULL) {
					break;
				}
				type = (c == '}' ? JSMNM_OBJECT : JSMNM_ARRAY);
#ifdef JSMNM_PARENT_LINKS
				if (parser->toknext < 1) {
#ifdef JSMNM_DEBUG
					printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
					return JSMNM_ERROR_INVAL;
				}
				token = &tokens[parser->toknext - 1];
				for (;;) {
					if (token->start != JSMN_NEG && token->end == JSMN_NEG) {
						if (token->type != type) {
#ifdef JSMNM_DEBUG
							printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
							return JSMNM_ERROR_INVAL;
						}
						token->end = parser->pos + 1;
						parser->toksuper = token->parent;
						break;
					}
					if (token->parent == JSMN_NEG) {
						if (token->type != type || parser->toksuper == JSMN_NEG) {
#ifdef JSMNM_DEBUG
							printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
							return JSMNM_ERROR_INVAL;
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
#ifdef JSMNM_DEBUG
							printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
							return JSMNM_ERROR_INVAL;
						}
						parser->toksuper = JSMN_NEG;
						token->end = parser->pos + 1;
						break;
					}
				}
				/* Error if unmatched closing bracket */
				if (i == -1) {
#ifdef JSMNM_DEBUG
					printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
					return JSMNM_ERROR_INVAL;
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
#ifdef JSMNM_DEBUG
				printf(" -> \"");
#endif
				r = jsmnm_parse_string(parser, js, len, tokens, num_tokens);
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
#ifdef JSMNM_DEBUG
				printf(" -> \\t \\r \\n");
#endif
				break;
			case ':':
#ifdef JSMNM_DEBUG
				printf(" -> :");
#endif
				parser->toksuper = parser->toknext - 1;
				break;
			case ',':
#ifdef JSMNM_DEBUG
				printf(" -> ,");
#endif
				if (tokens != NULL && parser->toksuper != JSMN_NEG &&
						tokens[parser->toksuper].type != JSMNM_ARRAY &&
						tokens[parser->toksuper].type != JSMNM_OBJECT) {
#ifdef JSMNM_PARENT_LINKS
					parser->toksuper = tokens[parser->toksuper].parent;
#else
					for (i = parser->toknext - 1; i >= 0; i--) {
						if (tokens[i].type == JSMNM_ARRAY || tokens[i].type == JSMNM_OBJECT) {
							if (tokens[i].start != JSMN_NEG && tokens[i].end == JSMN_NEG) {
								parser->toksuper = i;
								break;
							}
						}
					}
#endif
				}
				break;
#ifdef JSMNM_STRICT
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
#ifdef JSMNM_DEBUG
				printf(" -> 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, t, f, n");
#endif
				/* And they must not be keys of the object */
				if (tokens != NULL && parser->toksuper != JSMN_NEG) {
					t = &tokens[parser->toksuper];
					if (t->type == JSMNM_OBJECT ||
							(t->type == JSMNM_STRING && t->size != 0)) {
#ifdef JSMNM_DEBUG
						printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
						return JSMNM_ERROR_INVAL;
					}
				}
#else
			/* In non-strict mode every unquoted value is a primitive */
			default:
#endif
				r = jsmnm_parse_primitive(parser, js, len, tokens, num_tokens);
				if (r < 0) {
					return r;
				}
				count++;
				if (parser->toksuper != JSMN_NEG && tokens != NULL) {
					tokens[parser->toksuper].size++;
				}
				break;

#ifdef JSMNM_STRICT
			/* Unexpected char in strict mode */
			default:
#ifdef JSMNM_DEBUG
				printf(" -> JSMNM_ERROR_INVAL %d\n", __LINE__);
#endif
				return JSMNM_ERROR_INVAL;
#endif
		}
	}

	if (tokens != NULL) {
		for (i = parser->toknext - 1; i >= 0; i--) {
			/* Unmatched opened object or array */
			if (tokens[i].start != JSMN_NEG && tokens[i].end == JSMN_NEG) {
#ifdef JSMNM_DEBUG
				printf(" -> JSMNM_ERROR_PART %d\n", __LINE__);
#endif
				return JSMNM_ERROR_PART;
			}
		}
	}

#ifdef JSMNM_DEBUG
	printf(" -> count %d\n", __LINE__);
#endif
	return count;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
JSMNM_API void jsmnm_init(jsmnm_parser *parser) {
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = JSMN_NEG;
}

#endif /* JSMNM_HEADER */
