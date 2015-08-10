#ifndef STRING_TERM_MAP_H
#define STRING_TERM_MAP_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

struct term;

typedef char* string;

typedef struct{
    string key;
    struct term* val;
} string_term_kvp;

typedef struct{
    unsigned int size;
    int load;
    list **elements;
} string_term_map;

string_term_map *make_string_term_map();
void string_term_map_set(string_term_map *m, string key, struct term* val);
struct term *string_term_map_get(string_term_map *m, string key);

#endif
