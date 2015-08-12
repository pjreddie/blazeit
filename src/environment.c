#include "environment.h"
#include "term.h"
#include "term_list.h"

//djb2 hash function
//taken from http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash_string(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int compare_string(string s, string s2)
{
    return (strcmp(s, s2)==0);
}

environment *make_environment()
{
    environment *m = malloc(sizeof(environment));
    m->size = 63;
    m->load = 0;
    m->elements = calloc(m->size, sizeof(term_list*));
    return m;
}

void free_environment(environment *m)
{
    int i;
    for(i = 0; i < m->size; ++i){
        if(m->elements[i]){
            term_list *l = m->elements[i];
            while(l) l = pop_term_list(l);
        }
    }
    free(m->elements);
    free(m);
}

void expand_environment(environment *m)
{
    int i;
    int old_size = m->size;
    term_list **old_elements = m->elements;
    m->size = (m->size+1)*2-1;
    m->elements = calloc(m->size, sizeof(term_list*));
    for(i = 0; i < old_size; ++i){
        if(old_elements[i]){
            term_list *l = old_elements[i];
            while(l){
                term *t = l->value;
                unsigned int h = hash_string(t->name)%m->size;
                m->elements[h] = push_term_list(m->elements[h], t);
                l = pop_term_list(l);
            }
        }
    }
    free(old_elements);
}

void add_environment(environment *m, term* val)
{
    char *key = val->name;
    if((double)m->load/m->size > .7) expand_environment(m);  
    unsigned int h = hash_string(key)%m->size;
    term_list *l = m->elements[h];
    term_list *current = find_term_list(l, key);
    if(!current){
        m->elements[h] = push_term_list(l, val);
        ++m->load;
    }else{
        free_term(current->value);
        current->value = copy_term(val);
    }
}

term *get_environment(environment *m, string key)
{
    unsigned int h = hash_string(key)%m->size;
    term_list *l = find_term_list(m->elements[h], key);
    if(l && l->value->right) return l->value->right;
    return 0;
}

