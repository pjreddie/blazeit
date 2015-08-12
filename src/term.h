#ifndef TERM_H
#define TERM_H

struct environment;

typedef enum {
    VAR, APP, FUN, IND, PI, TYPE, CONS, DEF
} term_kind;

struct term;

typedef struct context{
    struct context *next;
    struct context *prev;
    struct term *value;
} context;

typedef struct term {
    term_kind kind;
    char *name;
    int n;

    struct term *left;
    struct term *right;
    struct term *annotation;
    struct term **constructors; 
} term;

void print_term(term *t);
term *copy_term(term *t);
void debruijn(term *t);
void free_term(term *t);
void replace_term(term *old, term *new);
int type_check(term *t, struct environment env, context *c, term *type);
term *type_infer(term *t, struct environment env, context *c);

struct environment evaluate_term(term *t, struct environment env);

#endif
