#ifndef TERM_H
#define TERM_H

typedef enum {
    ANN, VAR, APP, LAM, IND, FUN, TYPE, CONS
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

void evaluate_term(term *t);
void print_term(term *t);
term *copy_term(term *t);

#endif
