// https://github.com/alisdair/jsmn-example/

#include "json.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

const char *jsmn_strerror(enum jsmnerr errno)
{
    switch (errno) {
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

char *readJSONFile(const char *filename)
{
    char *jsonString = NULL;

    FILE *pFile;
    uint16_t lSize;
    size_t rv;

#ifndef NDEBUG
    printf("Opening for read: %s\n", filename);
#endif
    pFile = fopen(filename, "rb");
    if (pFile == NULL) {
        perror("fopen");
        return NULL;
    }

    // Obtain file size, first uint16_t of file
    fseek(pFile, 0, SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);

    // Allocate memory to contain the rest of the file
    jsonString = (char *)calloc(sizeof(char), lSize + 1);
    if (jsonString == NULL) {
        perror("calloc");
        fclose(pFile);
        return NULL;
    }

    // Copy the file into the jsonString
    rv = fread(jsonString, sizeof(char), lSize, pFile);
    if (rv != lSize) {
        perror("fread");
        free(jsonString);
        fclose(pFile);
        return NULL;
    } else {
        jsonString[lSize] = '\0';
    }

    // Close file
    fclose(pFile);

    return jsonString;
}

jsmntok_t *json_tokenize(char *json, size_t json_len, jsmnint_t *rv)
{
    jsmn_parser p;
    jsmn_init(&p);

    *rv = jsmn_parse(&p, json, json_len, NULL, 0);

    if (*rv < 0) {
#ifndef NPRINTF
        fprintf(stderr, "jsmn_parse: %s\n", jsmn_strerror(*rv));
#endif
        return NULL;
    }

#if !defined(NPRINTF) && !defined(NDEBUG)
    printf("jsmn_parse: %d tokens found.\n", *rv);
#endif

    jsmntok_t *tokens = (jsmntok_t *)calloc(sizeof(jsmntok_t), *rv);

    jsmn_init(&p);
    *rv = jsmn_parse(&p, json, json_len, tokens, *rv);

    return tokens;
}

jsmnint_t jsmnTokenLen(jsmntok_t *tok)
{
    return tok->end - tok->start;
}

int json_token_streq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

jsmnint_t isJSONKey(jsmntok_t *tokens, jsmnint_t t)
{
    if (tokens[t].type != JSMN_STRING)
        return JSMN_NEG; // JSON Key must be of type JSMN_STRING
    if (tokens[t].size != 1)
        return JSMN_NEG; // JSON Key can only have 1 child

    return 0;
}

jsmnint_t isJSONArrayMember(jsmntok_t *tokens, jsmnint_t t)
{
    if (tokens[t].parent == JSMN_NEG)
        return JSMN_NEG; // Must have a valid parent
    if (tokens[tokens[t].parent].type != JSMN_ARRAY)
        return JSMN_NEG; // Parent must be JSMN_ARRAY

    return 0;
}

jsmnint_t getJSONKeyValue(jsmntok_t *tokens, jsmnint_t t)
{
    jsmnint_t rv;
    if ((rv = isJSONKey(tokens, t)) != 0)
        return rv;

    return (t + 1);
}

jsmnint_t json_next_sibling(jsmntok_t *tokens, jsmnint_t t)
{
    // parent must be a JSMN_OBJECT or JSMN_ARRAY
    // parent's size must be > 1;
    // assume only one json string in string
    // from current token to end, look for another token with the same parent

#if defined(JSMN_NEXT_SIBLING)
    return tokens[t].next_sibling;
#elif defined(JSMN_PARENT_LINKS)
    // If token's parent isn't an object or array, return -1
    if (!(tokens[tokens[t].parent].type & (JSMN_OBJECT | JSMN_ARRAY)))
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

jsmnint_t json_parse_object(const char *json, jsmntok_t *tokens, jsmnint_t parent, const char *key)
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

jsmnint_t jsmn_parse_array(jsmntok_t *tokens, jsmnint_t parent, jsmnint_t key)
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

jsmnint_t json_parse(const char *json, jsmntok_t *tokens, uint32_t num_keys, ...)
{
    void *key[num_keys];
    jsmnint_t i, pos;

    // keys may be either const char * or jsmnint_t, at this point we don't care
    va_list keys;
    va_start(keys, num_keys);
    for (i = 0; i < num_keys; i++) {
        key[i] = va_arg(keys, void *);
    }
    va_end(keys);

    // start at position zero
    pos = 0;
    for (i = 0; i < num_keys; i++) {
        if (tokens[pos].type == JSMN_OBJECT) {
            // if `pos`.type is an object, treat key as a const char *
            pos = json_parse_object(json, tokens, pos, key[i]);
            // move position to current key's value (with checks)
            pos = getJSONKeyValue(tokens, pos);
        } else if (tokens[pos].type == JSMN_ARRAY) {
            // if `pos`.type is an array, treat key as a jsmnint_t (by way of uintptr_t)
            pos = jsmn_parse_array(tokens, pos, (jsmnint_t)((uintptr_t)key[i]));
        } else {
            // `pos` be either an object or array
            pos = JSMN_NEG;
            break;
        }

        // if json_parse_{object,array} returns JSMN_NEG, break
        if (pos == JSMN_NEG)
            break;
    }
    return pos;
}
