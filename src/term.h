#ifndef TERM_H
#define TERM_H

struct environment;

typedef enum {
    ANN, VAR, APP, FUN, IND, PI, TYPE, CONS, DEF
} term_kind;

struct term;

typedef struct term {
    term_kind kind;
    char *name;
    int n;

    struct term *left;
    struct term *right;
    struct term **constructors; 
} term;

void print_term(term *t);
term *copy_term(term *t);
void debruijn(term *t);
void free_term(term *t);

struct environment evaluate_term(term *t, struct environment env);

#endif
