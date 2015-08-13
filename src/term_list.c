#include "term_list.h"
#include "term.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

term *get_term_list(term_list *l, int n)
{
    int i;
    for(i = 0; i < n && l; ++i){
        l = l->next;
    }
    if(!l) return 0;
    term *lookup = l->value;
    //printf("Lookup: ");
    //print_term(lookup);
    //printf("\n");
    return lookup;
}

term_list *find_term_list(term_list *l, char *name)
{
    while(l){
        if(l->value->name && strcmp(name, l->value->name)==0) return l;
        l = l->next;
    }
    return 0;
}


term_list *push_term_list(term_list *l, term *t)
{
    term_list *new = calloc(1, sizeof(term_list));
    new->next = l;
    if(l) l->prev = new;
    new->value = copy_term(t);
    return new;
}

term_list *pop_term_list(term_list *l)
{
    free_term(l->value);
    term_list *next = l->next;
    if (next) next->prev = 0;
    free(l);
    return next;
}

void print_term_list(term_list *c)
{
    while(c){
        print_term(c->value);
        printf("...");
        c = c->next;
    }
    printf("\n");
}

