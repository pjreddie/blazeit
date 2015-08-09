#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "utils.h"

/*
    if (t->kind == ANN){
    }else if (t->kind == VAR){
    }else if (t->kind == APP){
    }else if (t->kind == FUN){
    }else if (t->kind == IND){
    }else if (t->kind == PI){
    }else if (t->kind == TYPE){
    }
*/
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
        if(t->n > level) t->n += start;
    }else if (t->kind == APP){
        increment(t->left, start, level);
        increment(t->right, start, level);
    }else if (t->kind == FUN){
        increment(t->left, start, level+1);
        increment(t->right, start, level+1);
    }else if (t->kind == IND){
    }else if (t->kind == PI){
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
    }else if (t->kind == FUN){
        //substitute(t->left, arg, level);
        substitute(t->right, arg, level+1);
    }else if (t->kind == IND){
    }else if (t->kind == PI){
    }else if (t->kind == TYPE){
    }
}

void free_term(term *t)
{
    free(t);
}

void evaluate_term(term *t)
{
    if (t->kind == FUN){
        evaluate_term(t->left);
        evaluate_term(t->right);
    }
    if (t->kind == APP){
        evaluate_term(t->left);
        evaluate_term(t->right);
        if (t->left->kind == FUN){
            
            term *lam = t->left;
            term *arg = t->right;
            term *body = copy_term(lam->right);
            substitute(body, arg, 0);
            free_term(t->left);
            free_term(t->right);
            *t = *body;
            free(body);

            evaluate_term(t);
        }
    }
}

typedef struct index_list{
    char *name;
    struct index_list *next;
} index_list;

void debruijn_r(term *t, index_list *list)
{
    if (t->kind == VAR){
        int count = 0;
        while(list){
            if(strcmp(list->name, t->name) == 0){
                t->n = count;
                break;
            }
            ++count;
            list = list->next;
        }
        t->n = count;
    }else if (t->kind == ANN){
    }else if (t->kind == APP){
        debruijn_r(t->left, list);
        debruijn_r(t->right, list);
    }else if (t->kind == FUN){
        index_list *top = calloc(1, sizeof(index_list));
        top->name = t->left->name;
        top->next = list;
        debruijn_r(t->right, top);
        free(top);
    }else if (t->kind == IND){
    }else if (t->kind == PI){
    }else if (t->kind == TYPE){
    }
}

void debruijn(term *t)
{
    debruijn_r(t, 0);
}

void print_term(term *t)
{
    if (t->kind == APP){
        printf("(");
        print_term(t->left);
        printf(" ");
        print_term(t->right);
        printf(")");
    }
    if(t->kind == FUN){
        printf("(fun ");
        print_term(t->left);
        printf(" => ");
        print_term(t->right);
        printf(")");
    }
    if(t->kind == VAR){
        printf("%s%d", t->name, t->n);
    }
}

