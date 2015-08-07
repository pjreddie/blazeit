#ifndef TERM_H
#define TERM_H

typedef enum {
    ANN, VAR, APP, LAM, IND, FUN, TYPE
} term_kind;

struct term;

typedef struct{
    struct term *left;
    struct term *right;
} application_term;

typedef struct{
    char *name;
    int n;
} variable_term;

typedef struct{
    struct term *e;
    struct term *type;
} annotation_term;

typedef struct{
    struct term *variable;
    struct term *body;
} lambda_term;




typedef struct constructor{
    char *name;
    struct term *t;
} constructor;

typedef struct inductive{
    int n;
    constructor *constructors;
} inductive_type;

typedef struct pi{
    struct term *left;
    struct term *right;
} pi_type;




typedef struct term {
    term_kind kind;

    application_term    application;
    variable_term       variable;
    annotation_term     annotation;
    lambda_term         lambda;

    inductive_type      inductive;
    pi_type             pi;

} term;

void evaluate_term(term *t);
void print_term(term *t);
term *copy_term(term *t);

#endif
