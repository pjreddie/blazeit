#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct term;
struct term_list;

typedef char* string;

typedef struct environment{
    unsigned int size;
    int load;
    struct term_list **elements;
} environment;

environment *make_environment();
void add_environment(environment *m, struct term *t);
struct term *get_environment(environment *m, string key);
void free_environment(environment *m);

#endif
