#include "jsmn.h"

#ifdef UNIT_TESTING
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#endif

#ifdef JSMN_DEBUG
#include <stdio.h>
#endif

/**
 * Allocates a fresh unused token from the token pull.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser,
		jsmntok_t *tokens, size_t num_tokens) {
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
static void jsmn_fill_token(jsmntok_t *token, jsmnenumtype_t type,
                            jsmnint_t start, jsmnint_t end) {
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
		size_t len, jsmntok_t *tokens, size_t num_tokens) {
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
			case '\t' : case '\r' : case '\n' : case ' ' :
			case ','  : case ']'  : case '}' :
				goto found;
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
		size_t len, jsmntok_t *tokens, size_t num_tokens) {

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
			jsmn_fill_token(token, JSMN_STRING, start+1, parser->pos);
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
				case '\"': case '\\' : case '/' : case 'b' :
				case 'f' : case 'n'  : case 'r' : case 't' :
					break;
				/* Allows escaped symbol \uXXXX */
				case 'u':
					parser->pos++;
					for(i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++) {
						/* If it isn't a hex character we have an error */
						if(!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
									(js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
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
jsmnint_t jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		     jsmntok_t *tokens, unsigned int num_tokens) {

	if (len >= JSMN_NEG) {
#ifdef JSMN_DEBUG
		printf(" -> JSMN_ERROR_LEN %d\n", __LINE__);
#endif
		return JSMN_ERROR_LEN;
	}

	int r;
	int i;
	jsmntok_t *token;
	jsmnint_t count = parser->toknext;

	char c;
	jsmnenumtype_t type;
	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		c = js[parser->pos];
#ifdef JSMN_DEBUG
		printf("\nJSON Position: %05d  Character: %c", parser->pos, c);
#endif
		switch (c) {
			case '{': case '[':
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
					tokens[parser->toksuper].size++;
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
			case '}': case ']':
#ifdef JSMN_DEBUG
				printf(" -> } or ]");
#endif
				if (tokens == NULL)
					break;
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
						if(token->type != type || parser->toksuper == JSMN_NEG) {
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
				if (r < 0) return r;
				count++;
				if (parser->toksuper != JSMN_NEG && tokens != NULL)
					tokens[parser->toksuper].size++;
				break;
			case '\t' : case '\r' : case '\n' : case ' ':
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
			case '-': case '0': case '1' : case '2': case '3' : case '4':
			case '5': case '6': case '7' : case '8': case '9':
			case 't': case 'f': case 'n' :
#ifdef JSMN_DEBUG
				printf(" -> 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, t, f, n");
#endif
				/* And they must not be keys of the object */
				if (tokens != NULL && parser->toksuper != JSMN_NEG) {
					jsmntok_t *t = &tokens[parser->toksuper];
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
				if (r < 0) return r;
				count++;
				if (parser->toksuper != JSMN_NEG && tokens != NULL)
					tokens[parser->toksuper].size++;
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
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser) {
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = JSMN_NEG;
}
