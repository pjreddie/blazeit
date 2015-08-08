#include <stdio.h>
#include <stdlib.h>
#include "term.h"
#include "utils.h"

term *copy_term(term *t)
{
    if(!t) return 0;
    term *copy = calloc(1, sizeof(term));
    copy->kind = t->kind;
    copy->left  = copy_term(t->left);
    copy->right = copy_term(t->right);
    copy->name = t->name;
    copy->n = t->n;
    if (t->kind == IND){
        not_implemented();
    }
    return copy;
}

void increment(term *t, int start, int level)
{
    if (t->kind == ANN){
    }else if (t->kind == VAR){
        if(t->n > start) t->n += level;
    }else if (t->kind == APP){
        increment(t->left, start, level);
        increment(t->right, start, level);
    }else if (t->kind == LAM){
        increment(t->left, start, level+1);
        increment(t->right, start, level+1);
    }else if (t->kind == IND){
    }else if (t->kind == FUN){
    }else if (t->kind == TYPE){
    }
}

void substitute(term *t, term *arg, int level)
{
    if (t->kind == ANN){
    }else if (t->kind == VAR){
        if(t->n == level){
            term *copy = copy_term(arg);       
            increment(copy, level, 0);
            *t = *copy;
            free(copy);
        }else if(t->n > level){
            t->n -= 1;
        }
    }else if (t->kind == APP){
        substitute(t->left, arg, level);
        substitute(t->right, arg, level);
    }else if (t->kind == LAM){
        //substitute(t->left, arg, level);
        substitute(t->right, arg, level+1);
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
        evaluate_term(t->left);
        evaluate_term(t->right);
    }
    if (t->kind == APP){
        evaluate_term(t->left);
        evaluate_term(t->right);
        if (t->left->kind == LAM){
            term *lam = t->left;
            term *arg = t->right;
            term *body = copy_term(lam->right);
            substitute(body, arg, 0);
            free_term(t->left);
            free_term(t->right);
            *t = *body;
            free(body);
        }
    }
}

void print_term(term *t)
{
    if (t->kind == APP){
        printf("(");
        print_term(t->left);
        print_term(t->right);
        printf(") ");
    }
    if(t->kind == LAM){
        printf("(fun ");
        print_term(t->left);
        printf(" => ");
        print_term(t->right);
        printf(") ");
    }
    if(t->kind == VAR){
        printf("%s%d", t->name, t->n);
    }
}

