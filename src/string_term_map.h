#ifndef STRING_TERM_MAP_H
#define STRING_TERM_MAP_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct term;
struct term_list;

typedef char* string;

typedef struct{
    unsigned int size;
    int load;
    struct term_list **elements;
} string_term_map;

string_term_map *make_string_term_map();
void string_term_map_add(string_term_map *m, struct term *t);
struct term *string_term_map_get(string_term_map *m, string key);
void free_string_term_map(string_term_map *m);

#endif
