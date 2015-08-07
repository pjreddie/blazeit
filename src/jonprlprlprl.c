#include <stdio.h>
#include <stdlib.h>
#include "term.h"

int main(int argc, char **argv)
{
    term *y = calloc(1, sizeof(term));   
    y->kind = VAR;
    y->variable.name = "y";
    y->variable.n = 4;
     
    term *ylam = calloc(1, sizeof(term));
    ylam->kind = LAM;
    ylam->lambda.variable = copy_term(y);
    ylam->lambda.body = copy_term(y);

    term *x = calloc(1, sizeof(term));   
    x->kind = VAR;
    x->variable.name = "x";
    x->variable.n = 0;

    term *lam = calloc(1, sizeof(term));;
    lam->kind = LAM;
    lam->lambda.variable = copy_term(x);
    lam->lambda.body = copy_term(x);
    
    term *app = calloc(1, sizeof(term));
    app->kind = APP;
    app->application.left = copy_term(lam);
    app->application.right = copy_term(ylam);

    print_term(app);
    printf("\n");
    evaluate_term(app);
    print_term(app);
    printf("\n");
    return 0;
}

