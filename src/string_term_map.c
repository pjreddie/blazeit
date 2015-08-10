#include "string_term_map.h"
#include "term.h"

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

string_term_map *make_string_term_map()
{
    string_term_map *m = malloc(sizeof(string_term_map));
    m->size = 63;
    m->load = 0;
    m->elements = calloc(m->size, sizeof(list*));
    return m;
}
void free_string_term_map(string_term_map *m)
{
    int i;
    for(i = 0; i < m->size; ++i){
        if(m->elements[i]){
            free_list_contents(m->elements[i]);
            free_list(m->elements[i]);
        }
    }
    free(m->elements);
    free(m);
}

void expand_string_term_map(string_term_map *m)
{
    int i;
    int old_size = m->size;
    list **old_elements = m->elements;
    m->size = (m->size+1)*2-1;
    m->elements = calloc(m->size, sizeof(list*));
    for(i = 0; i < old_size; ++i){
        if(old_elements[i]){
            list *l = old_elements[i];
            node *n = l->front;
            while(n != 0){
                string_term_kvp *pair = (string_term_kvp *)n->val;
                unsigned int h = hash_string(pair->key)%m->size;
                if(!m->elements[h]) m->elements[h] = make_list();
                list_insert(m->elements[h], pair);
                n = n->next;
            }
            free_list(l);
        }
    }
    free(old_elements);
}

string_term_kvp *string_term_kvp_list_find(list *l, string key)
{
    if(!l) return 0;
    node *n = l->front;
    while(n){
        string_term_kvp *pair = (string_term_kvp *)n->val;
        if(compare_string(pair->key, key)) return pair;
        n = n->next;
    }
    return 0;
}

void string_term_kvp_list_insert(list *l, string key, term* val)
{
        string_term_kvp *pair = malloc(sizeof(string_term_kvp));
        pair->key = key;
        pair->val = val;
        list_insert(l, pair);
}

void string_term_map_set(string_term_map *m, string key, term* val)
{
    if((double)m->load/m->size > .7) expand_string_term_map(m);  
    unsigned int h = hash_string(key)%m->size;
    list *l = m->elements[h];
    if(l == 0){
        m->elements[h] = make_list();
        string_term_kvp_list_insert(m->elements[h], key, val);
        ++m->load;
    }else{
        string_term_kvp *current = string_term_kvp_list_find(l, key);
        if(!current){
            string_term_kvp_list_insert(l, key, val);
            ++m->load;
        }else{
            free_term(current->val);
            current->val = val;
        }
    }
}

term *string_term_map_get(string_term_map *m, string key)
{
    unsigned int h = hash_string(key)%m->size;
    list *l = m->elements[h];
    string_term_kvp *pair = string_term_kvp_list_find(l, key);
    if(pair) return (pair->val);
    return 0;
}


