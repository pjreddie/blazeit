#ifndef TERM_H
#define TERM_H
#include "term_list.h"

struct environment;

typedef enum {
    VAR, APP, FUN, IND, PI, TYPE, CONS, DEF, HOLE, ELIM
} term_kind;

struct term;

typedef struct term {
    term_kind kind;
    char *name;
    int n;

    struct term *left;
    struct term *right;
    struct term *annotation;
    struct term **cases; 
} term;

term *copy_term(term *t);
void debruijn(term *t);
void free_term(term *t);
void replace_term(term *old, term *new);
int type_check(term *t, struct environment *env, term_list *context, term *type);
term *type_infer(term *t, struct environment *env, term_list *context);
void evaluate_term(term *t, struct environment *env);
term *make_eliminator(term *t, term **constructors, int n);
term *convert_unnamed(term *t);

void print_term(term *t);
void print_term_r(term *t, term_list *context);
#endif
