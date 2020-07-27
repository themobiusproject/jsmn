#ifndef JSMN_HEADER
#define JSMN_HEADER
#endif

#include "jsmn.h"

/**
 * Allocates a fresh unused token from the token pool.
 */
static
jsmntok_t *jsmn_alloc_token(jsmn_parser *parser, jsmntok_t *tokens,
                            const size_t num_tokens)
{
  if (parser->toknext >= num_tokens) {
    return NULL;
  }

  jsmntok_t *tok = &tokens[parser->toknext++];
  tok->start = tok->end = JSMN_NEG;
  tok->size = 0;
#if defined(JSMN_PARENT_LINKS)
  tok->parent = JSMN_NEG;
#endif
#if defined(JSMN_NEXT_SIBLING)
  tok->next_sibling = JSMN_NEG;
#endif
  parser->count++;
  return tok;
}

/**
 * Fills token type and boundaries.
 */
static
void jsmn_fill_token(jsmntok_t *token, const jsmntype_t type,
                     const jsmnint_t start, const jsmnint_t end)
{
  token->type = type;
  token->start = start;
  token->end = end;
  token->size = 0;
}

#if defined(JSMN_NEXT_SIBLING)
/**
 * Set previous child's next_sibling to current token
 */
static
void jsmn_next_sibling(jsmn_parser *parser, jsmntok_t *tokens)
{
  jsmnint_t sibling;

  /* Start with parent's first child */
  if (parser->toksuper != JSMN_NEG) {
    sibling = parser->toksuper + 1;
  } else {
    sibling = 0;
  }

  /* If the first child is the current token */
  if (sibling == parser->toknext - 1) {
    return;
  }

  /* Loop until we find previous sibling */
  while (tokens[sibling].next_sibling != JSMN_NEG) {
    sibling = tokens[sibling].next_sibling;
  }

  /* Set previous sibling's next_sibling to current token */
  tokens[sibling].next_sibling = parser->toknext - 1;
}
#endif

static
jsmnbool isWhitespace(const char c)
{
  if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
    return JSMN_TRUE;
  }
  return JSMN_FALSE;
}

static
jsmnbool isHexadecimal(const char c)
{
  if ((c >= '0' && c <= '9') ||
      (c >= 'A' && c <= 'F') ||
      (c >= 'a' && c <= 'f')) {
    return JSMN_TRUE;
  }
  return JSMN_FALSE;
}

static
jsmnbool isCharacter(const char c)
{
  if (c >= ' ' && c != '"' && c != '\\') {
    return JSMN_TRUE;
  }
  return JSMN_FALSE;
}

static
jsmnbool isSpecialChar(const char c)
{
  if (c == '{' || c == '}' || c == '[' || c == ']' ||
      c == '"' || c == ':' || c == ',') {
    return JSMN_TRUE;
  }
  return JSMN_FALSE;
}

/**
 * Fills next available token with JSON primitive.
 */
static
jsmnint_t jsmn_parse_primitive(jsmn_parser *parser, const char *js,
                               const size_t len, jsmntok_t *tokens,
                               const size_t num_tokens)
{
  /* If a PRIMITIVE wasn't expected */
  if (!(parser->expected & (JSMN_PRIMITIVE | JSMN_PRI_CONTINUE))) {
    return JSMN_ERROR_INVAL;
  }

  jsmnint_t pos;
  jsmntype_t type;
  jsmntype_t expected = JSMN_CLOSE;

  if (!(parser->expected & JSMN_PRI_CONTINUE)) {
    pos = parser->pos;
  } else {
    if (tokens != NULL) {
      pos = tokens[parser->toknext - 1].start;
    } else {
      pos = parser->pos;
      while (pos != JSMN_NEG &&
             !isWhitespace(js[pos]) &&
             !isSpecialChar(js[pos]) &&
             isCharacter(js[pos])) {
        pos--;
      }
      pos++;
    }
  }
  type = JSMN_PRIMITIVE;

#if !defined(JSMN_PERMISSIVE_PRIMITIVE)
# if !defined(JSMN_PERMISSIVE_LITERALS)
  char literal[][6] = { "true", "false", "null" };
# else
  char literal[][9] = { "true", "false", "null", "NaN", "Infinity" };
# endif
  jsmnint_t i;
  for (i = 0; i < sizeof(literal) / sizeof(literal[0]); i++) {
    if (js[pos] != literal[i][0]) {
      continue;
    }
    jsmnint_t j;
    for (j = 1, pos++; literal[i][j] != '\0'; j++, pos++) {
      if (pos == len ||
          js[pos] == '\0') {
        return JSMN_ERROR_PART;
      }
      if (js[pos] != literal[i][j]) {
        return JSMN_ERROR_INVAL;
      }
    }
    type |= JSMN_PRI_LITERAL;
    if (pos == len) {
      goto found;
    }
    goto check_primitive_border;
  }

  expected = JSMN_PRI_MINUS | JSMN_PRI_INTEGER;
  for (; pos < len; pos++) {
    if (js[pos] == '0') {
      if (!(expected & JSMN_PRI_INTEGER)) {
        return JSMN_ERROR_INVAL;
      }
      if (type & JSMN_PRI_EXPONENT) {
        expected = JSMN_PRI_INTEGER |                                        JSMN_CLOSE;
      } else if (type & JSMN_PRI_DECIMAL) {
        expected = JSMN_PRI_INTEGER |                    JSMN_PRI_EXPONENT | JSMN_CLOSE;
      } else if (parser->pos == pos ||
                 (parser->pos + 1 == pos && (type & JSMN_PRI_MINUS))) {
        expected =                    JSMN_PRI_DECIMAL | JSMN_PRI_EXPONENT | JSMN_CLOSE;
      } else {
        expected = JSMN_PRI_INTEGER | JSMN_PRI_DECIMAL | JSMN_PRI_EXPONENT | JSMN_CLOSE;
      }
      continue;
    }

    if (js[pos] >= '1' && js[pos] <= '9') {
      if (!(expected & JSMN_PRI_INTEGER)) {
        return JSMN_ERROR_INVAL;
      }
      if (type & JSMN_PRI_EXPONENT) {
        expected = JSMN_PRI_INTEGER |                                        JSMN_CLOSE;
      } else if (type & JSMN_PRI_DECIMAL) {
        expected = JSMN_PRI_INTEGER |                    JSMN_PRI_EXPONENT | JSMN_CLOSE;
      } else {
        expected = JSMN_PRI_INTEGER | JSMN_PRI_DECIMAL | JSMN_PRI_EXPONENT | JSMN_CLOSE;
      }
      continue;
    }

    if (js[pos] == '-') {
      if (!(expected & JSMN_PRI_MINUS)) {
        return JSMN_ERROR_INVAL;
      }
      if (parser->pos == pos) {
        type |= JSMN_PRI_MINUS;
      }
      expected = JSMN_PRI_INTEGER;
      continue;
    }

    if (js[pos] == '+') {
      if (!(expected & JSMN_PRI_SIGN)) {
        return JSMN_ERROR_INVAL;
      }
      expected = JSMN_PRI_INTEGER;
      continue;
    }

    if (js[pos] == '.') {
      if (!(expected & JSMN_PRI_DECIMAL)) {
        return JSMN_ERROR_INVAL;
      }
      type |= JSMN_PRI_DECIMAL;
      expected = JSMN_PRI_INTEGER;
      continue;
    }

    if (js[pos] == 'e' || js[pos] == 'E') {
      if (!(expected & JSMN_PRI_EXPONENT)) {
        return JSMN_ERROR_INVAL;
      }
      type |= JSMN_PRI_EXPONENT;
      expected = JSMN_PRI_SIGN | JSMN_PRI_INTEGER;
      continue;
    }

    if (!(expected & JSMN_CLOSE)) {
      return JSMN_ERROR_INVAL;
    }
    goto check_primitive_border;
  }
  if (!(expected & JSMN_CLOSE)) {
    return JSMN_ERROR_INVAL;
  }
  goto found;

check_primitive_border:
  switch (js[pos]) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':
  case ',':
  case '}':
  case ']':
  case '\0':
    goto found;
  case '"':
  case ':':
  case '{':
  case '[':
  default:
    return JSMN_ERROR_INVAL;
  }
#else
  for (; pos < len && js[pos] != '\0'; pos++) {
    if (isWhitespace(js[pos]) ||
        isSpecialChar(js[pos])) {
      goto found;
    }
    if (!isCharacter(js[pos])) {
      return JSMN_ERROR_INVAL;
    }
  }
#endif

found:
  expected = parser->expected;
  if (parser->toksuper != JSMN_NEG) {
#if defined(JSMN_PERMISSIVE_KEY)
    /* OBJECT KEY, strict query */
    if ((expected & (JSMN_KEY | JSMN_INSD_OBJ)) == (JSMN_KEY | JSMN_INSD_OBJ)) {
      parser->expected = JSMN_AFTR_OBJ_KEY;
      type |= JSMN_KEY   | JSMN_INSD_OBJ;
    } else
#endif
    /* OBJECT VALUE, VALUE is implicit */
    if (expected & JSMN_INSD_OBJ) {
      parser->expected = JSMN_AFTR_OBJ_VAL;
      type |= JSMN_VALUE | JSMN_INSD_OBJ;
    /* ARRAY VALUE, VALUE is implicit */
    } else {
      parser->expected = JSMN_AFTR_ARR_VAL;
      type |= JSMN_VALUE;
    }
  } else {
    parser->expected = JSMN_ROOT;
#if defined(JSMN_PERMISSIVE_RULESET) && defined(JSMN_MULTIPLE_JSON_FAIL)
    if (expected & JSMN_KEY) {
      parser->expected |= JSMN_COLON;
    }
#endif
    type |= JSMN_VALUE;
  }
  if (pos == len) {
    parser->expected |= JSMN_PRI_CONTINUE;
  }

  if (tokens == NULL) {
    parser->pos = pos - 1;
    if (!(expected & JSMN_PRI_CONTINUE)) {
      parser->count++;
    }
    return JSMN_SUCCESS;
  }

  jsmntok_t *token;
  if (!(expected & JSMN_PRI_CONTINUE)) {
    token = jsmn_alloc_token(parser, tokens, num_tokens);
    if (token == NULL) {
      parser->expected = expected;
      return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, type, parser->pos, pos);
  } else {
    token = &tokens[parser->toknext - 1];
    jsmn_fill_token(token, type, token->start, pos);
  }
  parser->pos = pos;
#if defined(JSMN_PARENT_LINKS)
  token->parent = parser->toksuper;
#endif
#if defined(JSMN_NEXT_SIBLING)
  jsmn_next_sibling(parser, tokens);
#endif

  if (parser->toksuper != JSMN_NEG) {
    if (!(expected & JSMN_PRI_CONTINUE)) {
      tokens[parser->toksuper].size++;
    }

    if (!(tokens[parser->toksuper].type & JSMN_CONTAINER)) {
#if defined(JSMN_PARENT_LINKS)
      parser->toksuper = tokens[parser->toksuper].parent;
#else
      jsmnint_t i;
      for (i = parser->toksuper; i != JSMN_NEG; i--) {
        if (tokens[i].type & JSMN_CONTAINER && tokens[i].end == JSMN_NEG) {
          parser->toksuper = i;
          break;
        }
      }
# if defined(JSMN_PERMISSIVE_RULESET)
      if (i == JSMN_NEG) {
        parser->toksuper = i;
      }
# endif
#endif
    }
  }
  parser->pos--;

  return JSMN_SUCCESS;
}

/**
 * Fills next token with JSON string.
 */
static
jsmnint_t jsmn_parse_string(jsmn_parser *parser, const char *js,
                            const size_t len, jsmntok_t *tokens,
                            const size_t num_tokens)
{
  /* If a STRING wasn't expected */
  if (!(parser->expected & JSMN_STRING)) {
    return JSMN_ERROR_INVAL;
  }

  jsmnint_t pos = parser->pos;

  /* Skip starting quote */
  pos++;

  char c;
  for (; pos < len && js[pos] != '\0'; pos++) {
    c = js[pos];

    /* Quote: end of string */
    if (c == '"') {
      jsmntype_t expected = parser->expected;
      jsmntype_t type;
      if (parser->toksuper != JSMN_NEG) {
        /* OBJECT KEY, strict query */
        if ((expected & (JSMN_INSD_OBJ | JSMN_KEY)) == (JSMN_INSD_OBJ | JSMN_KEY)) {
          parser->expected = JSMN_AFTR_OBJ_KEY;
          type = JSMN_STRING | JSMN_KEY   | JSMN_INSD_OBJ;
        /* OBJECT VALUE, VALUE is implicit */
        } else if (expected & JSMN_INSD_OBJ) {
          parser->expected = JSMN_AFTR_OBJ_VAL;
          type = JSMN_STRING | JSMN_VALUE | JSMN_INSD_OBJ;
        /* ARRAY VALUE, VALUE is implicit */
        } else {
          parser->expected = JSMN_AFTR_ARR_VAL;
          type = JSMN_STRING | JSMN_VALUE;
        }
      } else {
        parser->expected = JSMN_ROOT;
#if defined(JSMN_PERMISSIVE_RULESET) && defined(JSMN_MULTIPLE_JSON_FAIL)
        if (expected & JSMN_KEY) {
          parser->expected |= JSMN_COLON;
        }
#endif
        type = JSMN_STRING | JSMN_VALUE;
      }

      if (tokens == NULL) {
        parser->pos = pos;
        parser->count++;
        return JSMN_SUCCESS;
      }

      jsmntok_t *token = jsmn_alloc_token(parser, tokens, num_tokens);
      if (token == NULL) {
        parser->expected = expected;
        return JSMN_ERROR_NOMEM;
      }
      jsmn_fill_token(token, type, parser->pos + 1, pos);
      parser->pos = pos;
#if defined(JSMN_PARENT_LINKS)
      token->parent = parser->toksuper;
#endif
#if defined(JSMN_NEXT_SIBLING)
      jsmn_next_sibling(parser, tokens);
#endif

      if (parser->toksuper != JSMN_NEG) {
        tokens[parser->toksuper].size++;

        if (!(tokens[parser->toksuper].type & JSMN_CONTAINER)) {
#if defined(JSMN_PARENT_LINKS)
          parser->toksuper = tokens[parser->toksuper].parent;
#else
          jsmnint_t i;
          for (i = parser->toksuper; i != JSMN_NEG; i--) {
            if (tokens[i].type & JSMN_CONTAINER && tokens[i].end == JSMN_NEG) {
              parser->toksuper = i;
              break;
            }
          }
# if defined(JSMN_PERMISSIVE_RULESET)
          if (i == JSMN_NEG) {
            parser->toksuper = i;
          }
# endif
#endif
        }
      }

      return JSMN_SUCCESS;
    }

    /* Backslash: Quoted symbol expected */
    if (c == '\\' && pos + 1 < len) {
      pos++;
      switch (js[pos]) {
      /* Allowed escaped symbols */
      case '"':
      case '\\':
      case '/':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
        break;
      /* Allows escaped symbol \uhhhh */
      case 'u':
        pos++;
        jsmnint_t i;
        for (i = pos + 4; pos < i; pos++) {
          if (pos == len ||
              js[pos] == '\0') {
            return JSMN_ERROR_PART;
          }
          /* If it isn't a hex character we have an error */
          if (!isHexadecimal(js[pos])) {
            return JSMN_ERROR_INVAL;
          }
        }
        pos--;
        break;
#if defined(JSMN_PERMISSIVE_UTF32)
      /* Allows escaped symbol \Uhhhhhhhh */
      case 'U':
        pos++;
        for (i = pos + 8; pos < i; pos++) {
          if (pos == len ||
              js[pos] == '\0') {
            return JSMN_ERROR_PART;
          }
          /* If it isn't a hex character we have an error */
          if (!isHexadecimal(js[pos])) {
            return JSMN_ERROR_INVAL;
          }
        }
        pos--;
        break;
#endif
      /* Unexpected symbol */
      default:
        return JSMN_ERROR_INVAL;
      }
    }

    /* form feed, new line, carraige return, tab, and vertical tab not allowed */
    else if (c == '\f' ||
             c == '\n' ||
             c == '\r' ||
             c == '\t' ||
             c == '\v') {
      return JSMN_ERROR_INVAL;
    }
  }
  return JSMN_ERROR_PART;
}

static
jsmnint_t jsmn_parse_container_open(jsmn_parser *parser, const char c,
                                    jsmntok_t *tokens, const size_t num_tokens)
{
  /* If an OBJECT or ARRAY wasn't expected */
  if (!(parser->expected & JSMN_CONTAINER)) {
    return JSMN_ERROR_INVAL;
  }

  jsmntype_t type;
  if (c == '{') {
    parser->expected = JSMN_OPEN_OBJECT;
    type = JSMN_OBJECT | JSMN_VALUE;
  } else {
    parser->expected = JSMN_OPEN_ARRAY;
    type = JSMN_ARRAY  | JSMN_VALUE;
  }

  if (tokens == NULL) {
    parser->toksuper++;
    if (parser->toksuper < (sizeof(jsmnint_t) * CHAR_BIT) &&
        parser->expected & JSMN_INSD_OBJ) {
      parser->toknext |= (1 << parser->toksuper);
    }
    parser->count++;
    return JSMN_SUCCESS;
  }

  if (parser->toksuper != JSMN_NEG &&
      tokens[parser->toksuper].type & JSMN_INSD_OBJ) {
    type |= JSMN_INSD_OBJ;
  }

  jsmntok_t *token = jsmn_alloc_token(parser, tokens, num_tokens);
  if (token == NULL) {
    return JSMN_ERROR_NOMEM;
  }
  jsmn_fill_token(token, type, parser->pos, JSMN_NEG);
#if defined(JSMN_PARENT_LINKS)
  token->parent = parser->toksuper;
#endif
#if defined(JSMN_NEXT_SIBLING)
  jsmn_next_sibling(parser, tokens);
#endif

  if (parser->toksuper != JSMN_NEG) {
    tokens[parser->toksuper].size++;
  }
  parser->toksuper = parser->toknext - 1;

  return JSMN_SUCCESS;
}

static
jsmnint_t jsmn_parse_container_close(jsmn_parser *parser, const char c,
                                     jsmntok_t *tokens)
{
  /* If an OBJECT or ARRAY CLOSE wasn't expected */
  if (!(parser->expected & JSMN_CLOSE)) {
    return JSMN_ERROR_INVAL;
  }

  if (tokens == NULL) {
    if (parser->toksuper < (sizeof(jsmnint_t) * CHAR_BIT)) {
      jsmntype_t type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
      if ((((parser->toknext & (1 << parser->toksuper)) == 1) && !(type & JSMN_OBJECT)) ||
          (((parser->toknext & (1 << parser->toksuper)) == 0) && !(type & JSMN_ARRAY))) {
        return JSMN_ERROR_BRACKETS;
      }
      parser->toknext &= ~(1 << parser->toksuper);
    }
    parser->toksuper--;
  } else {
#if defined(JSMN_PERMISSIVE_RULESET)
    if (parser->toksuper == JSMN_NEG) {
      return JSMN_ERROR_BRACKETS;
    }
#endif
    jsmntype_t type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
    jsmntok_t *token = &tokens[parser->toksuper];

    if (!(token->type & type) ||
        token->end != JSMN_NEG) {
      return JSMN_ERROR_BRACKETS;
    }
    token->end = parser->pos + 1;
#if defined(JSMN_PARENT_LINKS)
    if (token->type & JSMN_INSD_OBJ &&
        !(tokens[token->parent].type & JSMN_CONTAINER)) {
      parser->toksuper = tokens[token->parent].parent;
    } else {
      parser->toksuper = token->parent;
    }
#else
    jsmnint_t i;
    for (i = parser->toksuper - 1; i != JSMN_NEG; i--) {
      if (tokens[i].type & JSMN_CONTAINER && tokens[i].end == JSMN_NEG) {
        parser->toksuper = i;
        break;
      }
    }
    if (i == JSMN_NEG) {
      parser->toksuper = i;
    }
#endif
  }

  if (parser->toksuper != JSMN_NEG) {
    parser->expected = JSMN_AFTR_CLOSE;
  } else {
    parser->expected = JSMN_ROOT;
  }

  return JSMN_SUCCESS;
}

static
jsmnint_t jsmn_parse_colon(jsmn_parser *parser, jsmntok_t *tokens)
{
  /* If a COLON wasn't expected */
  if (!(parser->expected & JSMN_COLON)) {
    return JSMN_ERROR_INVAL;
  }

  if (parser->toksuper != JSMN_NEG) {
    parser->expected = JSMN_AFTR_COLON;
#if defined(JSMN_PERMISSIVE_RULESET)
  } else {
    parser->expected = JSMN_AFTR_COLON_R;
#endif
  }

  if (tokens == NULL) {
    return JSMN_SUCCESS;
  }

#if defined(JSMN_PERMISSIVE_RULESET)
  tokens[parser->toknext - 1].type &= ~JSMN_VALUE;
  tokens[parser->toknext - 1].type |= JSMN_KEY;
#endif

  parser->toksuper = parser->toknext - 1;

  return JSMN_SUCCESS;
}

static
jsmnint_t jsmn_parse_comma(jsmn_parser *parser, jsmntok_t *tokens)
{
  /* If a COMMA wasn't expected */
  if (!(parser->expected & JSMN_COMMA)) {
    return JSMN_ERROR_INVAL;
  }

  jsmntype_t type = JSMN_UNDEFINED;
  if (tokens == NULL) {
    if (parser->toksuper < (sizeof(jsmnint_t) * CHAR_BIT) &&
        parser->toknext & (1 << parser->toksuper)) {
      type = JSMN_INSD_OBJ;
    }
  } else {
    if (parser->toksuper != JSMN_NEG) {
      type = tokens[parser->toksuper].type;
    }
  }

  if (parser->toksuper != JSMN_NEG) {
    if (type & (JSMN_OBJECT | JSMN_INSD_OBJ)) {
      parser->expected = JSMN_AFTR_COMMA_O;
    } else {
      parser->expected = JSMN_AFTR_COMMA_A;
    }
#if defined(JSMN_PERMISSIVE_RULESET)
  } else {
    parser->expected = JSMN_AFTR_COMMA_R;
#endif
  }

  if (tokens == NULL) {
    return JSMN_SUCCESS;
  }

#if defined(JSMN_PERMISSIVE_RULESET)
  tokens[parser->toknext - 1].type |= JSMN_VALUE;
#endif

  return JSMN_SUCCESS;
}

/**
 * Parse JSON string and fill tokens.
 */
JSMN_API
jsmnint_t jsmn_parse(jsmn_parser *parser, const char *js,
                     const size_t len, jsmntok_t *tokens,
                     const size_t num_tokens)
{
  if (((jsmnint_t)-1 > 0 && len >= (jsmnint_t)JSMN_ERROR_MAX) ||
      len > JSMNINT_MAX) {
    return JSMN_ERROR_LENGTH;
  }

  jsmnint_t r;

  for (; parser->pos < len; parser->pos++) {
#if !defined(JSMN_MULTIPLE_JSON_FAIL)
    if (parser->expected == JSMN_UNDEFINED) {
      break;
    }
#endif
    char c = js[parser->pos];
    if (c == '{' || c == '[') {
      r = jsmn_parse_container_open(parser, c, tokens, num_tokens);
      if (r != JSMN_SUCCESS) {
        return r;
      }
      continue;
    }

    if (c == '}' || c == ']') {
      r = jsmn_parse_container_close(parser, c, tokens);
      if (r != JSMN_SUCCESS) {
        return r;
      }
      continue;
    }

    if (c == '"') {
      r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
      if (r != JSMN_SUCCESS) {
        return r;
      }
      continue;
    }

    if (c == ':') {
      r = jsmn_parse_colon(parser, tokens);
      if (r != JSMN_SUCCESS) {
        return r;
      }
      continue;
    }

    if (c == ',') {
      r = jsmn_parse_comma(parser, tokens);
      if (r != JSMN_SUCCESS) {
        return r;
      }
      continue;
    }

    /* Valid whitespace */
    if (isWhitespace(c)) {
      continue;
    }

#if !defined(JSMN_PERMISSIVE_PRIMITIVE)
    /* rfc8259: PRIMITIVEs are numbers and booleans */
    if (c == '-' || (c >= '0' && c <= '9') ||
        c == 'n' ||  c == 't' || c == 'f') {
#else
    /* In permissive mode every unquoted value is a PRIMITIVE */
    if (isCharacter(c) || c == '\\') {
#endif
      r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
      if (r != JSMN_SUCCESS) {
        return r;
      }
      continue;
    }

    /* Unexpected char */
    return JSMN_ERROR_INVAL;
  }

  if (parser->toksuper != JSMN_NEG) {
    return JSMN_ERROR_PART;
  }

  if (parser->count == 0) {
    return JSMN_ERROR_INVAL;
  }

  while (parser->pos < len && isWhitespace(js[parser->pos])) {
    parser->pos++;
  }

  return parser->count;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
JSMN_API
void jsmn_init(jsmn_parser *parser)
{
  parser->pos = 0;
  parser->toknext = 0;
  parser->toksuper = JSMN_NEG;
  parser->count = 0;
  parser->expected = JSMN_ROOT_INIT;
}
