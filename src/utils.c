#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "utils.h"

void file_error(char *s)
{
    fprintf(stderr, "Couldn't open file: %s\n", s);
    exit(0);
}

char *copy_string(char *s)
{
    int len = strlen(s);
    char *copy = calloc(len+1, sizeof(char));
    memcpy(copy, s, len);
    return copy;
}

void error(const char *s)
{
    fprintf(stderr, "Error: %s\n", s);
    exit(-1);
}

void malloc_error()
{
    fprintf(stderr, "Malloc error\n");
    exit(-1);
}

void not_implemented()
{
    fprintf(stderr, "Not implemented.\n");
    assert(0);
}

char *fgetl(FILE *fp)
{
    if(feof(fp)) return 0;
    size_t size = 512;
    char *line = malloc(size*sizeof(char));
    if(!fgets(line, size, fp)){
        free(line);
        return 0;
    }

    size_t curr = strlen(line);

    while((line[curr-1] != '\n') && !feof(fp)){
        if(curr == size-1){
            size *= 2;
            line = realloc(line, size*sizeof(char));
            if(!line) {
                printf("%ld\n", size);
                malloc_error();
            }
        }
        size_t readsize = size-curr;
        if(readsize > INT_MAX) readsize = INT_MAX-1;
        fgets(&line[curr], readsize, fp);
        curr = strlen(line);
    }
    if(line[curr-1] == '\n') line[curr-1] = '\0';

    return line;
}

