/*

ind nat : Type0 = 
| 0 : nat
| S : nat -> nat
.

def plus : (_:nat) -> (_:nat) -> nat =
    fun x => fun y =>
        nat.ind (fun _:nat => nat) y (fun _:nat => fun sum:nat => S sum) x.


TERM:   def V = E
        TYPE
        FUN
        VAR
        (TERM)

TYPE:   var -> term ...

FUN:    fun TERM => TERM

APP:    TERM TERM ...

VAR:    <var> : TERM
        TYPE

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "utils.h"
#include "tokenizer.h"

void expect(token_kind kind, token_list **l)
{
    token_list *token = *l;
    if(!token || token->kind != kind){
        printf("Expected: ");
        print_token(kind);
        printf("\n");

        print_tokens(*l);
        error("Syntax Error");
    }
    (*l) = (*l)->next;
}

int accept(token_kind kind, token_list **l)
{
    token_list *token = *l;
    if(!token || token->kind != kind) return 0;
    (*l) = (*l)->next;
    return 1;
}

int peek(token_kind kind, token_list **l)
{
    token_list *token = *l;
    return (token && token->kind == kind);
}

term *convert_unnamed(term *t)
{
    if(t->annotation) return t;
    term *type = t;
    term *var = calloc(1, sizeof(term));
    var->kind = VAR;
    var->annotation = type;
    var->name = copy_string("_");
    return var;
}

term *parse_pi(token_list **list, term *t)
{
    term *pi = calloc(1, sizeof(term));
    pi->kind = PI;
    pi->left = convert_unnamed(t);
    pi->right = parse(list);
    return pi;
}

term *parse_application(token_list **list, term *t)
{
    term *arg = parse_term(list);
    while(arg){
        term *app = calloc(1, sizeof(term));
        app->kind = APP;
        app->left = t;
        app->right = arg;
        t = app;
        arg = parse_term(list);
    }
    return t;
}

term *parse_def(token_list **list)
{
    term *t = calloc(1, sizeof(term));
    t->kind = DEF;
    t->left = parse(list);
    t->name = copy_string(t->left->name);
    expect(EQUAL_T, list);
    t->right = parse(list);
    return t;
}

term *parse_ind(token_list **list)
{
    term *t = calloc(1, sizeof(term));
    t->kind = IND;
    t->left = parse(list);
    t->name = copy_string(t->left->name);
    t->constructors = calloc(1, sizeof(term*));
    expect(EQUAL_T, list);
    accept(OR_T, list);
    int count = 0;
    t->constructors[count++] = parse(list);
    while(accept(OR_T, list)){
        t->constructors = realloc(t->constructors, (count+1)*sizeof(term*));
        t->constructors[count++] = parse(list);
    }
    t->n = count;
    return t;
}

term *parse_type(token_list **list)
{
    term *t = calloc(1, sizeof(term));
    t->kind = TYPE;
    return t;
}

term *parse_var(token_list **list)
{
    if(accept(OPEN_T, list)){
        term *sub = parse_var(list);
        expect(CLOSE_T, list);
        return sub;
    }

    token_list *token = *list;
    if (!accept(VAR_T, list)) expect(UNDER_T, list);
    term *t = calloc(1, sizeof(term));
    t->kind = VAR;
    t->name = copy_string(token->value);
    if(accept(COLON_T, list)){
        t->annotation = parse_term(list);
    }
    return t;
}

term *parse_fun(token_list **list)
{
    term *top = calloc(1, sizeof(term));
    term *t = top;
    t->kind = FUN;
    t->left = parse_var(list);
    while(1){
        if (accept(ARR_T, list)) break;
        if(accept(COLON_T, list)){
            t->annotation = parse(list);
            expect(ARR_T, list);
            break;
        }
        term *next = calloc(1, sizeof(term));
        t->right = next;
        next->left = parse_var(list);
        next->kind = FUN;
        t = next;

    }
    t->right = parse(list);
    return top;
}

term *parse_subterm(token_list **list)
{
    term *t = parse(list);
    expect(CLOSE_T, list);
    return t;
}

term *parse_hole(token_list **list)
{
    term *t = calloc(1, sizeof(term));
    t->kind = HOLE;
    if(accept(COLON_T, list)){
        t->annotation = parse_term(list);
    }
    return t;
}

term *parse_term(token_list **list)
{
    if (accept(OPEN_T, list)) return parse_subterm(list);
    if (accept(FUN_T, list)) return parse_fun(list);
    if (peek(VAR_T, list) || peek(UNDER_T,list)) return parse_var(list);
    if (accept(DEF_T, list)) return parse_def(list);
    if (accept(IND_T, list)) return parse_ind(list);
    if (accept(TYPE_T, list)) return parse_type(list);
    if (accept(HOLE_T, list)) return parse_hole(list);
    return 0;
}

term *parse(token_list **list)
{
    term *t = parse_term(list);
    if(!(*list)) return t;
    if(!t) return 0;
    t = parse_application(list, t);
    if(accept(TO_T, list)) return parse_pi(list, t);
    return t;
}

term *parse_string(char *s)
{
    token_list *tokens = tokenize(s);
    //print_tokens(tokens);
    token_list *start = tokens;
    term *t = parse(&tokens);
    free_tokens(start);
    debruijn(t);
    return t;
}

