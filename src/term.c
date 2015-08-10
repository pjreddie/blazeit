#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "utils.h"
#include "environment.h"

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
    if (t->name) copy->name = copy_string(t->name);
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
            if (t->name) free(t->name);
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
    if(!t) return;
    free_term(t->left);
    free_term(t->right);
    if (t->name) free(t->name);
    free(t);
}

void replace_term(term *old, term *new)
{
    term *copy = copy_term(new);
    free_term(old->left);
    free_term(old->right);
    if (old->name) free(old->name);
    *old = *copy;
    free(copy);
}

environment evaluate_term(term *t, environment env)
{
    if (t->kind == DEF){
        set_environment(env, t);
    }
    if (t->kind == VAR){
        term *lookup = 0;
        if (t->name) lookup = get_environment(env, t->name);
        if (lookup){
            replace_term(t, lookup);
            evaluate_term(t, env);
        }
    }
    if (t->kind == FUN){
        evaluate_term(t->left, env);
        evaluate_term(t->right, env);
    }
    if (t->kind == APP){
        evaluate_term(t->left, env);
        evaluate_term(t->right, env);
        if (t->left->kind == FUN){
            
            term *lam = t->left;
            term *arg = copy_term(t->right);

            replace_term(t, lam->right);

            substitute(t, arg, 0);

            free_term(arg);
            evaluate_term(t, env);
        }
    }
    return env;
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
                free(t->name);
                t->name = 0;
                break;
            }
            ++count;
            list = list->next;
        }
        t->n = count;
    }else if (t->kind == ANN){
    }else if (t->kind == DEF){
        debruijn_r(t->right, list);
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

void print_term_r(term *t, index_list *list)
{
    if (t->kind == VAR){
        if(t->name){
            printf("%s", t->name);
            return;
        }
        int i;
        for(i = 0; i< t->n; ++i){
            if(!list) break;
            list = list->next;
        }
        if(!list){
            printf("%s", t->name);
        } else {
            printf("%s", list->name);
        }
    }else if (t->kind == ANN){
        printf("(");
        print_term_r(t->left, list);
        printf(" : ");
        print_term_r(t->right, list);
        printf(")");
    }else if (t->kind == DEF){
        printf("def ");
        print_term_r(t->left, list);
        printf(" = ");
        print_term_r(t->right, list);
    }else if (t->kind == APP){
        printf("(");
        print_term_r(t->left, list);
        printf(" ");
        print_term_r(t->right, list);
        printf(")");
    }else if (t->kind == FUN){
        printf("(fun ");
        print_term_r(t->left, list);
        printf(" => ");

        index_list *top = calloc(1, sizeof(index_list));
        top->name = t->left->name;
        top->next = list;
        print_term_r(t->right, top);
        free(top);

        printf(")");
    }else if (t->kind == IND){
    }else if (t->kind == PI){
    }else if (t->kind == TYPE){
    }
}

void print_term(term *t)
{
    print_term_r(t, 0);
}
/* void print_term(term *t)
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
   if(t->kind == DEF){
   printf("def ");
   print_term(t->left);
   printf(" = ");
   print_term(t->right);
   }
   if(t->kind == ANN){
   printf("(");
   print_term(t->left);
   printf(" : ");
   print_term(t->right);
   printf(")");
   }
   if(t->kind == VAR){
   if(t->name){
   printf("%s%d", t->name, t->n);
   }else{
   printf("%d", t->n);
   }
   }
   }
 */

