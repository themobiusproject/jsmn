#define JSMN_HEADER
#include "../json.h"

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    FILE *fp;
    size_t size;
    char *JSON_STRING;

    fp = fopen("../../library.json", "r");

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    JSON_STRING = malloc(sizeof(char) * size);
    fread(JSON_STRING, sizeof(char), size, fp);

    fclose(fp);

    explodeJSON(JSON_STRING, size);

    free(JSON_STRING);

    explodeJSON("{\"a\"}",             sizeof("{\"a\"}"));
    explodeJSON("{\"a\": 1, \"b\"}",   sizeof("{\"a\": 1, \"b\"}"));
    explodeJSON("{\"a\",\"b\":1}",     sizeof("{\"a\",\"b\":1}"));
    explodeJSON("{\"a\":1,}",          sizeof("{\"a\":1,}"));
    explodeJSON("{\"a\":\"b\":\"c\"}", sizeof("{\"a\":\"b\":\"c\"}"));

    return EXIT_SUCCESS;
}
