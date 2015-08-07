#include <stdio.h>
#include <stdlib.h>
#include "term.h"
#include "utils.h"

/*
    if (t->kind == ANN){
    }else if (t->kind == VAR){
    }else if (t->kind == APP){
    }else if (t->kind == LAM){
    }else if (t->kind == IND){
    }else if (t->kind == FUN){
    }else if (t->kind == TYPE){
    }
*/

term *copy_term(term *t)
{
    term *copy = calloc(1, sizeof(term));
    copy->kind = t->kind;
    if (t->kind == ANN){
        copy->annotation.e = copy_term(t->annotation.e);
        copy->annotation.type = copy_term(t->annotation.type);
    }else if (t->kind == VAR){
        copy->variable.name = t->variable.name;
        copy->variable.n = t->variable.n;
    }else if (t->kind == APP){
        copy->application.left = copy_term(t->application.left);
        copy->application.right = copy_term(t->application.right);
    }else if (t->kind == LAM){
        copy->lambda.variable = copy_term(t->lambda.variable);
        copy->lambda.body = copy_term(t->lambda.body);
    }else if (t->kind == IND){
        not_implemented();
    }else if (t->kind == FUN){
        not_implemented();
    }else if (t->kind == TYPE){
        not_implemented();
    }
    return copy;
}

void increment(term *t, int start, int level)
{
    if (t->kind == ANN){
    }else if (t->kind == VAR){
        if(t->variable.n > start) t->variable.n += level;
    }else if (t->kind == APP){
        increment(t->application.left, start, level);
        increment(t->application.right, start, level);
    }else if (t->kind == LAM){
        increment(t->lambda.variable, start, level+1);
        increment(t->lambda.body, start, level+1);
    }else if (t->kind == IND){
    }else if (t->kind == FUN){
    }else if (t->kind == TYPE){
    }
}

void substitute(term *t, term *arg, int level)
{
    if (t->kind == ANN){
    }else if (t->kind == VAR){
        if(t->variable.n == level){
            term *copy = copy_term(arg);       
            increment(copy, level, 0);
            *t = *copy;
            free(copy);
        }else if(t->variable.n > level){
            t->variable.n -= 1;
        }
    }else if (t->kind == APP){
        substitute(t->application.left, arg, level);
        substitute(t->application.right, arg, level);
    }else if (t->kind == LAM){
        //substitute(t->lambda.variable, arg, level);
        substitute(t->lambda.body, arg, level+1);
    }else if (t->kind == IND){
    }else if (t->kind == FUN){
    }else if (t->kind == TYPE){
    }
}

void free_term(term *t)
{
    free(t);
}

void evaluate_term(term *t)
{
    if (t->kind == LAM){
        evaluate_term(t->lambda.body);
    }
    if (t->kind == APP){
        evaluate_term(t->application.left);
        evaluate_term(t->application.right);
        if (t->application.left->kind == LAM){
            term *lam = t->application.left;
            term *body = copy_term(lam->lambda.body);
            term *arg = t->application.right;
            substitute(body, arg, 0);
            free_term(t->application.left);
            free_term(t->application.right);
            *t = *body;
            free(body);
        }
    }
}

void print_term(term *t)
{
    if (t->kind == APP){
        printf("(");
        print_term(t->application.left);
        print_term(t->application.right);
        printf(") ");
    }
    if(t->kind == LAM){
        printf("(fun ");
        print_term(t->lambda.variable);
        printf(" => ");
        print_term(t->lambda.body);
        printf(") ");
    }
    if(t->kind == VAR){
        printf("%s%d", t->variable.name, t->variable.n);
    }
}

