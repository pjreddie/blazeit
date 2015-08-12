#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "string_term_map.h"

struct term;

typedef struct environment{
    string_term_map *map;
} environment;

environment make_environment();
void set_environment(environment env, struct term *t);
struct term *get_environment(environment env, char *s);
void free_environment(environment env);

#endif

