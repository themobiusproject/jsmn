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

    JSON_STRING = calloc(size, sizeof(char));
    fread(JSON_STRING, sizeof(char), size, fp);

    fclose(fp);

    explodeJSON(JSON_STRING, size);

    free(JSON_STRING);

    return EXIT_SUCCESS;
}
