#include "environment.h"
#include "term.h"

environment make_environment()
{
    environment env;
    env.map = make_string_term_map();
    return env;
}

void free_environment(environment env)
{
    free_string_term_map(env.map);
}

void set_environment(environment env, term *t)
{
    string_term_map_add(env.map, t);
}

term *get_environment(environment env, char *s)
{
    term *t = string_term_map_get(env.map, s);
    if(!t) return 0;
    return t->right;
}

