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

#include "dbgprintf.h"

#if (defined(__linux__) || defined(__APPLE__) || defined(ARDUINO))
#define EXPORT __attribute__ ((visibility ("default")))
#else
#define EXPORT
#endif

#ifndef NDEBUG
#define inline
#endif

EXPORT
const char *jsmnm_strerror(jsmnmenumtype_t errno)
{
    switch (errno) {
        case JSMNM_ERROR_NOMEM:
            return "Not enough tokens were provided.";
        case JSMNM_ERROR_INVAL:
            return "Invalid character inside JSON string.";
        case JSMNM_ERROR_PART:
            return "The string is not a full JSON packet, more bytes expected.";
        case JSMNM_ERROR_LEN:
            return "Input data too long.";
    }

    return NULL;
}

EXPORT
jsmnmtok_t *json_tokenize(const char *json, size_t json_len, jsmnmint_t *rv)
{
    jsmnm_parser p;
    jsmnm_init(&p);

    *rv = jsmnm_parse(&p, json, json_len, NULL, 0);

    // enum jsmnmer has four errors, thus
    if (*rv + 4 < 4) {
        dbgprintf("jsmnm_parse error: %s\n", jsmnm_strerror(*rv));
        return NULL;
    }

//     dbgprintf("jsmnm_parse: %d tokens found.\n", *rv);

    jsmnmtok_t *tokens = calloc(*rv, sizeof(jsmnmtok_t));

    jsmnm_init(&p);
    *rv = jsmnm_parse(&p, json, json_len, tokens, *rv);

    return tokens;
}

EXPORT
jsmnmint_t json_tokenize_noalloc(jsmnmtok_t *tokens, uint32_t num_tokens, const char *json, size_t json_len)
{
    jsmnm_parser p;
    jsmnm_init(&p);

    jsmnmint_t rv;

    rv = jsmnm_parse(&p, json, json_len, tokens, num_tokens);

    // enum jsmnmer has four errors, thus
    if (rv + 4 < 4) {
        dbgprintf("jsmnm_parse error: %s\n", jsmnm_strerror(rv));
        return rv;
    }

//     dbgprintf("jsmnm_parse: %d tokens found.\n", rv);

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
int json_token_streq(const char *json, const jsmnmtok_t *tok, const char *s)
{
    if (tok->type == JSMNM_STRING && strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

static inline
jsmnmint_t isJSONKey(const jsmnmtok_t *tokens, const jsmnmint_t t)
{
    if (tokens[t].type != JSMNM_STRING)
        return JSMN_NEG; // JSON Key must be of type JSMNM_STRING
    if (tokens[t].size != 1)
        return JSMN_NEG; // JSON Key can only have 1 child

    return 0;
}

static inline
jsmnmint_t getJSONKeyValue(const jsmnmtok_t *tokens, const jsmnmint_t t)
{
    if (isJSONKey(tokens, t) == JSMN_NEG)
        return JSMN_NEG;

    return (t + 1);
}

static inline
jsmnmint_t json_next_sibling(const jsmnmtok_t *tokens, const jsmnmint_t t)
{
    // parent must be a JSMNM_OBJECT or JSMNM_ARRAY
    // parent's size must be > 1;
    // assume only one json string in string
    // from current token to end, look for another token with the same parent

#if defined(JSMNM_NEXT_SIBLING)
    return tokens[t].next_sibling;
#elif defined(JSMNM_PARENT_LINKS)
    // If token's parent isn't an object or array, return -1
    if (!(tokens[tokens[t].parent].type & (JSMNM_OBJECT | JSMNM_ARRAY)))
        return JSMN_NEG;

    // If token's parent only has one child, return -1
    if (tokens[tokens[t].parent].size == 1)
        return JSMN_NEG;

    jsmnmint_t i, child_num = 1;

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
jsmnmint_t json_parse_object(const char *json, const jsmnmtok_t *tokens, const jsmnmint_t parent, const char *key)
{
    // first child is the first token after the parent
    jsmnmint_t child = parent + 1;

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
jsmnmint_t json_parse_array(const jsmnmtok_t *tokens, const jsmnmint_t parent, const jsmnmint_t key)
{
    // if parent's size is less than or equal to key, key is bad
    if (tokens[parent].size <= key)
        return JSMN_NEG;

    // first child is the first token after the parent
    jsmnmint_t i, child = parent + 1;
    // loop through children until you reach the nth child
    for (i = 0; i < key; i++) {
        child = json_next_sibling(tokens, child);
    }

    // return nth child
    return child;
}

EXPORT
jsmnmint_t json_parse(const char *json, const jsmnmtok_t *tokens, const size_t num_keys, ...)
{
    jsmnmint_t i, pos;

    // keys may be either const char * or jsmnmint_t, at this point we don't care
    va_list keys;
    va_start(keys, num_keys);

    // start at position zero
    pos = 0;
    for (i = 0; i < num_keys; i++) {
        if (tokens[pos].type == JSMNM_OBJECT) {
            // if `pos`.type is an object, treat key as a const char *
            if ((pos = json_parse_object(json, tokens, pos, va_arg(keys, void *))) == JSMN_NEG) break;
            // move position to current key's value (with checks)
            pos = getJSONKeyValue(tokens, pos);
        } else if (tokens[pos].type == JSMNM_ARRAY) {
            // if `pos`.type is an array, treat key as a jsmnmint_t (by way of uintptr_t)
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
void explodeJSON(const char *json, size_t len)
{
#ifndef NPRINTF
    jsmnmint_t rv, i;

    jsmnmtok_t *tokens = json_tokenize(json, len, &rv);

    if (rv + 4 < 4) {
        printf("jsmnm_parse error: %s\n", jsmnm_strerror(rv));
        return;
    }

//     const char *jsmnmtype[] = { "UNDEFINED", "OBJECT", "ARRAY", "", "STRING", "", "", "", "PRIMITIVE", };
    const char *jsmnmtype[] = { "UND", "OBJ", "ARR", "", "STR", "", "", "", "PRI", };

    for (i = 0; i < rv; i++) {
#ifdef JSMNM_PARENT_LINKS
        if (tokens[i].parent == JSMN_NEG)
            printf("\n");
#endif
        printf("Token %3d :  type: %3s |  start: %4d |  end: %4d |  length: %4d |  size : %2d",
               i, jsmnmtype[(jsmnmenumtype_t)tokens[i].type], tokens[i].start, tokens[i].end, tokens[i].end - tokens[i].start, tokens[i].size);
#ifdef JSMNM_PARENT_LINKS
        printf(" |  parent: %3d", (tokens[i].parent != JSMN_NEG ? tokens[i].parent : -1));
#endif
#ifdef JSMNM_NEXT_SIBLING
        printf(" |  sibling: %3d", (tokens[i].next_sibling != JSMN_NEG ? tokens[i].next_sibling : -1));
#endif
        printf(" | ");

        if (tokens[i].type == JSMNM_OBJECT) {
            printf("{");
        }
        if (tokens[i].type == JSMNM_ARRAY) {
            printf("[");
        }

        if (tokens[i].type == JSMNM_STRING && tokens[i].size == 1) {
            printf("\"%.*s\" :", tokens[i].end - tokens[i].start, &json[tokens[i].start]);
        }
        if (tokens[i].size == 0) {
            printf("    ");
            if (tokens[i].type == JSMNM_STRING)
                printf("\"");
            printf("%.*s", tokens[i].end - tokens[i].start, json + tokens[i].start);
            if (tokens[i].type == JSMNM_STRING)
                printf("\"");
            printf(",");
        }
        printf("\n");

        if (tokens[i].size != 0)
            continue;

        if (tokens[i].parent == JSMN_NEG)
            continue;

        if (tokens[i].next_sibling != JSMN_NEG || tokens[tokens[i].parent].next_sibling != JSMN_NEG)
            continue;

        if (tokens[i].size == 0 && tokens[i].type & (JSMNM_STRING | JSMNM_PRIMITIVE) && tokens[tokens[i].parent].next_sibling == JSMN_NEG)
            printf("          :                                                                                                    | ");

        if (tokens[tokens[i].parent].type == JSMNM_ARRAY)
            printf("]");
        else if (tokens[tokens[tokens[i].parent].parent].type == JSMNM_OBJECT)
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
