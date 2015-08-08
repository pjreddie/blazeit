#include <stdio.h>
#include <stdlib.h>
#include "term.h"

int main(int argc, char **argv)
{
    term *y = calloc(1, sizeof(term));   
    y->kind = VAR;
    y->name = "y";
    y->n = 4;
     
    term *ylam = calloc(1, sizeof(term));
    ylam->kind = LAM;
    ylam->left = copy_term(y);
    ylam->right = copy_term(y);

    term *x = calloc(1, sizeof(term));   
    x->kind = VAR;
    x->name = "x";
    x->n = 0;

    term *lam = calloc(1, sizeof(term));;
    lam->kind = LAM;
    lam->left = copy_term(x);
    lam->right = copy_term(x);
    
    term *app = calloc(1, sizeof(term));
    app->kind = APP;
    app->left = copy_term(lam);
    app->right = copy_term(ylam);

    print_term(app);
    printf("\n");
    evaluate_term(app);
    print_term(app);
    printf("\n");
    return 0;
}

