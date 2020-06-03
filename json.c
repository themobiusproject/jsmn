#include "json.h"

#ifdef UNIT_TESTING
#include <setjmp.h>
#include <cmocka.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#if (defined(__linux__) || defined(__APPLE__) || defined(ARDUINO))
#define EXPORT __attribute__ ((visibility ("default")))
#else
#define EXPORT
#endif

#ifndef NDEBUG
#define inline
#endif

EXPORT
const char *jsmn_strerror(enum jsmnerr errno)
{
    switch (errno) {
        case JSMN_SUCCESS:
            return "*** Success, should not be printing an error. ***";
        case JSMN_ERROR_NOMEM:
            return "Not enough tokens were provided.";
        case JSMN_ERROR_INVAL:
            return "Invalid character inside JSON string.";
        case JSMN_ERROR_PART:
            return "The string is not a full JSON packet, more bytes expected.";
        case JSMN_ERROR_LEN:
            return "Input data too long.";
    }

    return NULL;
}

EXPORT
jsmntok_t *json_tokenize(const char *json, const size_t json_len, jsmnint_t *rv)
{
    jsmn_parser p;
    jsmn_init(&p);

    *rv = jsmn_parse(&p, json, json_len, NULL, 0);

    // enum jsmnerr has four errors, thus
    if (*rv + 4 < 4) {
        fprintf(stderr, "jsmn_parse error: %s\n", jsmn_strerror(*rv));
        return NULL;
    }

//  fprintf(stderr, "jsmn_parse: %d tokens found.\n", *rv);

    jsmntok_t *tokens = calloc(*rv, sizeof(jsmntok_t));

    jsmn_init(&p);
    *rv = jsmn_parse(&p, json, json_len, tokens, *rv);

    return tokens;
}

EXPORT
jsmnint_t json_tokenize_noalloc(jsmntok_t *tokens, const uint32_t num_tokens, const char *json, const size_t json_len)
{
    jsmn_parser p;
    jsmn_init(&p);

    jsmnint_t rv;

    rv = jsmn_parse(&p, json, json_len, tokens, num_tokens);

    // enum jsmnerr has four errors, thus
    if (rv + 4 < 4) {
        fprintf(stderr, "jsmn_parse error: %s\n", jsmn_strerror(rv));
        return rv;
    }

//  fprintf(stderr, "jsmn_parse: %d tokens found.\n", rv);

    return rv;
}

/**
 * @brief String comparison between token and string
 *
 * @param[in] json JSON String
 * @param[in] tok Token to compare
 * @param[in] s String to complare
 * @return 0 when token string and s are equal, -1 otherwise
 */
static inline
int json_token_streq(const char *json, const jsmntok_t *tok, const char *s)
{
    if (jsmn_is_type(tok, JSMN_STRING) && strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return JSMN_SUCCESS;
    }
    return -1;
}

static inline
jsmnint_t getJSONKeyValue(const jsmntok_t *tokens, const jsmnint_t t)
{
    if (jsmn_is_type(&tokens[t], JSMN_KEY) == 0)
        return JSMN_NEG;

    return (t + 1);
}

static inline
jsmnint_t json_next_sibling(const jsmntok_t *tokens, const jsmnint_t t)
{
    // parent must be a JSMN_OBJECT or JSMN_ARRAY
    // parent's size must be > 1;
    // assume only one json string in string
    // from current token to end, look for another token with the same parent

#if defined(JSMN_NEXT_SIBLING)
    return tokens[t].next_sibling;
#elif defined(JSMN_PARENT_LINKS)
    // If token's parent isn't an OBJECT or ARRAY, return -1
    if (!jsmn_is_type(&tokens[tokens[t].parent], JSMN_OBJECT | JSMN_ARRAY))
        return JSMN_NEG;

    // If token's parent only has one child, return -1
    if (tokens[tokens[t].parent].size == 1)
        return JSMN_NEG;

    jsmnint_t i, child_num = 1;

    // Figure out what child number token is
    for (i = tokens[t].parent + 1; i < t; i++) {
        if (tokens[i].parent == tokens[t].parent) {
            child_num++;
        }
    }

    // If child number is the same as parent's size, then token is the last child
    if (child_num == tokens[tokens[t].parent].size)
        return JSMN_NEG;

    i = t + 1;
    while (tokens[i].parent != tokens[t].parent) {
        i++;
    }

    return i;
#else
    // Figure it out yourself
    return JSMN_NEG;
#endif
}

static inline
jsmnint_t json_parse_object(const char *json, const jsmntok_t *tokens, const jsmnint_t parent, const char *key)
{
    // first child is the first token after the parent
    jsmnint_t child = parent + 1;

    // loop through children
    while (child != JSMN_NEG) {
        // if child's string is equal to key
        if (json_token_streq(json, &tokens[child], key) == 0) {
            // return current child
            return child;
        }

        // move to the next child
        child = json_next_sibling(tokens, child);
    }

    // key didn't match any of the json keys
    return JSMN_NEG;
}

static inline
jsmnint_t json_parse_array(const jsmntok_t *tokens, const jsmnint_t parent, const jsmnint_t key)
{
    // if parent's size is less than or equal to key, key is bad
    if (tokens[parent].size <= key)
        return JSMN_NEG;

    // first child is the first token after the parent
    jsmnint_t i, child = parent + 1;
    // loop through children until you reach the nth child
    for (i = 0; i < key; i++) {
        child = json_next_sibling(tokens, child);
    }

    // return nth child
    return child;
}

EXPORT
jsmnint_t json_parse(const char *json, const jsmntok_t *tokens, const size_t num_keys, ...)
{
    jsmnint_t i, pos;

    // keys may be either const char * or jsmnint_t, at this point we don't care
    va_list keys;
    va_start(keys, num_keys);

    // start at position zero
    pos = 0;
    for (i = 0; i < num_keys; i++) {
        if (jsmn_is_type(&tokens[pos], JSMN_OBJECT)) {
            // if `pos`.type is an object, treat key as a const char *
            if ((pos = json_parse_object(json, tokens, pos, va_arg(keys, void *))) == JSMN_NEG) break;
            // move position to current key's value (with checks)
            pos = getJSONKeyValue(tokens, pos);
        } else if (jsmn_is_type(&tokens[pos], JSMN_ARRAY)) {
            // if `pos`.type is an array, treat key as a jsmnint_t (by way of uintptr_t)
            pos = json_parse_array(tokens, pos, (uintptr_t)va_arg(keys, void *));
        } else {
            // `pos` must be either an object or array
            pos = JSMN_NEG;
            break;
        }

        // if json_parse_{object,array} returns JSMN_NEG, break
        if (pos == JSMN_NEG) {
            break;
        }
    }

    va_end(keys);
    return pos;
}

EXPORT
void explodeJSON(const char *json, const size_t len)
{
#ifndef NPRINTF
    jsmnint_t rv, i;

    jsmntok_t *tokens = json_tokenize(json, len, &rv);
    jsmntok_t *token;

    if (rv + 4 < 4) {
        printf("jsmn_parse error: %s\n", jsmn_strerror(rv));
        return;
    }

    const char *jsmntype[] = { "", "OBJ", "ARR", "", "STR", "", "", "", "PRI", };
    const char *jsmnextr[] = { "", "KEY", "VAL", "", "CLS", "", "", "", "DLM", };

    printf("\n");
    for (i = 0; i < rv; i++) {
        token = &tokens[i];
        printf("Token %3d :  type: %3s |  start: %4d |  end: %4d |  length: %4d |  size : %2d",
               i, jsmntype[token->type & JSMN_ANY_TYPE], token->start, token->end, token->end - token->start, token->size);
#ifdef JSMN_PARENT_LINKS
        printf(" |  parent: %3d", (token->parent != JSMN_NEG ? token->parent : -1));
#endif
#ifdef JSMN_NEXT_SIBLING
        printf(" |  sibling: %3d", (token->next_sibling != JSMN_NEG ? token->next_sibling : -1));
#endif
        if (jsmn_is_type(token, JSMN_KEY | JSMN_VALUE)) {
            printf(" | %3s", jsmnextr[(token->type & (JSMN_KEY | JSMN_VALUE)) >> 4]);
        } else {
            printf(" |    ");
        }
        printf(" | ");

        if (jsmn_is_type(token, JSMN_OBJECT)) {
            printf("{");
        }
        if (jsmn_is_type(token, JSMN_ARRAY)) {
            printf("[");
        }

        if (jsmn_is_type(token, JSMN_STRING) && token->size == 1) {
            printf("\"%.*s\" :", token->end - token->start, &json[token->start]);
        }
        if (token->size == 0) {
            printf("    ");
            if (jsmn_is_type(token, JSMN_STRING))
                printf("\"");
            printf("%.*s", token->end - token->start, json + token->start);
            if (jsmn_is_type(token, JSMN_STRING))
                printf("\"");
// TODO Only print comma if (parent is JSMN_ARRAY && next_sibling != JSMN_NEG) || (grandparent is JSMN_OBJECT && parent next_sibling != JSMN_NEG)
            printf(",");
        }
        printf("\n");

        if (token->size != 0)
            continue;

        if (token->parent == JSMN_NEG)
            continue;

        if (token->next_sibling != JSMN_NEG || tokens[token->parent].next_sibling != JSMN_NEG)
            continue;

        if (token->size == 0 && jsmn_is_type(token, JSMN_STRING | JSMN_PRIMITIVE) && tokens[token->parent].next_sibling == JSMN_NEG)
            printf("          :            |              |            |               |           ");
#ifdef JSMN_PARENT_LINKS
            printf(" |             ");
#endif
#ifdef JSMN_NEXT_SIBLING
            printf(" |              ");
#endif
            printf(" |    ");
            printf(" | ");

        if (jsmn_is_type(&tokens[token->parent], JSMN_ARRAY))
            printf("]");
        else if (jsmn_is_type(&tokens[tokens[token->parent].parent], JSMN_OBJECT))
            printf("}");

        printf("\n");
    }

    free(tokens);
#endif // NPRINTF
}

EXPORT
void explodeJSON_nolen(const char *json)
{
    explodeJSON(json, strlen(json));
}
