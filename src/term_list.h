#ifndef TERM_LIST_H
#define TERM_LIST_H

struct term;

typedef struct term_list{
    struct term_list *next;
    struct term_list *prev;
    struct term *value;
} term_list;

struct term *get_term_list(term_list *l, int n);
term_list *find_term_list(term_list *l, char *name);
term_list *push_term_list(term_list *l, struct term *t);
term_list *pop_term_list(term_list *l);

#endif
