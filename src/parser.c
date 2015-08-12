/*

ind nat : Type0 = 
| 0 : nat
| S : nat -> nat
.

def plus : (_:nat) -> (_:nat) -> nat =
    fun x => fun y =>
        nat.ind (fun _:nat => nat) y (fun _:nat => fun sum:nat => S sum) x.

term:   (fun <term> => <term>)
        (<term> <term>)
        (<term> : <term>)
        (<term> -> <term>)
        <var>

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "utils.h"

typedef enum {
    NONE_T, OPEN_T, CLOSE_T, FUN_T, VAR_T, ARR_T, TO_T, DEF_T, COLON_T, EQUAL_T, TYPE_T, UNDER_T
} token_kind;

void print_token(token_kind kind)
{
    switch (kind){
        case NONE_T:
            printf("NONE");
            break;
        case OPEN_T:
            printf("( ");
            break;
        case TYPE_T:
            printf("TYPE ");
            break;
        case CLOSE_T:
            printf(") ");
            break;
        case FUN_T:
            printf("fun ");
            break;
        case VAR_T:
            printf("<var> ");
            break;
        case ARR_T:
            printf("=> ");
            break;
        case TO_T:
            printf("-> ");
            break;
        case DEF_T:
            printf("def ");
            break;
        case COLON_T:
            printf(": ");
            break;
        case EQUAL_T:
            printf("= ");
            break;
        case UNDER_T:
            printf("_ ");
            break;
    }
}

typedef struct token_list{
    struct token_list *next;
    struct token_list *prev;
    token_kind kind;
    char *value;
} token_list;

void free_tokens(token_list *list)
{
    while(list){
        token_list *next = list->next;
        if(list->value) free(list->value);
        free(list);
        list = next;
    }
}

int is_varchar(char s)
{
    return (s >= 65 && s <= 90) || (s >= 97 && s <= 122) || (s >= 48 && s <= 57) || s == '_';
}

token_list *tokenize(char *s)
{
    token_list *start = calloc(1, sizeof(token_list));
    start->kind = NONE_T;

    token_list *curr = start;
    token_list *next;
    int i = 0;
    //int len = strlen(s);
    while(*s){
        if(curr->kind != NONE_T){
            next = calloc(1, sizeof(token_list));
            next->prev = curr;
            next->kind = NONE_T;
            curr->next = next;
            curr = next;
        }
        if (*s == '('){
            curr->kind = OPEN_T;
            ++s;
            ++i;
            continue;
        }
        if (*s == ')'){
            curr->kind = CLOSE_T;
            ++s;
            ++i;
            continue;
        }
        if (*s == '_'){
            curr->kind = UNDER_T;
            ++s;
            ++i;
            continue;
        }
        if (*s == ':'){
            curr->kind = COLON_T;
            ++s;
            ++i;
            continue;
        }
        if (*s == 'f'){
            if (strncmp(s, "fun", 3) == 0){
                curr->kind = FUN_T;
                s += 3;
                i += 3;
                continue;
            }
        }
        if (*s == 'T'){
            if (strncmp(s, "Type", 4) == 0){
                curr->kind = TYPE_T;
                s += 4;
                i += 4;
                continue;
            }
        }
        if (*s == 'd'){
            if (strncmp(s, "def", 3) == 0){
                curr->kind = DEF_T;
                s += 3;
                i += 3;
                continue;
            }
        }
        if (*s == '='){
            if (strncmp(s, "=>", 2) == 0){
                curr->kind = ARR_T;
                s += 2;
                i += 2;
                continue;
            }else{
                curr->kind = EQUAL_T;
                ++s;
                ++i;
                continue;
            }
        }
        if (*s == '-'){
            if (strncmp(s, "->", 2) == 0){
                curr->kind = TO_T;
                s += 2;
                i += 2;
                continue;
            }
        }
        if (is_varchar(*s)){
            int j = 0;
            while(is_varchar(s[j])) ++j;
            char *copy = calloc(j+1, sizeof(char));
            memcpy(copy, s, j);
            curr->kind = VAR_T;
            curr->value = copy;
            s += j;
            i += j;
            continue;
        }
        if (*s == ' ' || *s == '\t'){
            ++s;
            ++i;
            continue;
        }
        printf("Didn't recognize char '%c'\n", *s);
        ++s;
        ++i;
    }
    return start;
}

void print_tokens(token_list *list)
{
    while (list){
        print_token(list->kind);
        list = list->next;
    }
    printf("\n");
}

void expect(token_kind kind, token_list **l)
{
    token_kind k = (*l)->kind;
    if(k != kind){
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
    token_kind k = (*l)->kind;
    if(k != kind) return 0;
    (*l) = (*l)->next;
    return 1;
}

/*
term:   (fun <term> => <term>)
        (<term> <term>)
        (<term> : <term>)
        (<term> -> <term>)
        <var>

*/

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

term *parse(token_list **list)
{
    token_list *token = *list;
    term *t = calloc(1, sizeof(term));

    if (accept(DEF_T, list)){
        t->kind = DEF;
        t->left = parse(list);
        expect(EQUAL_T, list);
        t->right = parse(list);
    } else if (accept(VAR_T, list)){
        t->kind = VAR;
        t->name = copy_string(token->value);
        if(accept(COLON_T, list)){
            t->annotation = parse(list);
        }
    } else if (accept(UNDER_T, list)){
        t->kind = VAR;
        t->name = copy_string("_");
        if(accept(COLON_T, list)){
            t->annotation = parse(list);
        }
    } else if (accept(TYPE_T, list)){
        t->kind = TYPE;
    } else {
        expect(OPEN_T, list);
        if (accept(FUN_T, list)){
            term *start = t;
            t->kind = FUN;
            t->left = parse(list);
            while(1){
                if (accept(ARR_T, list)) break;
                if(accept(COLON_T, list)){
                    t->annotation = parse(list);
                    expect(ARR_T, list);
                    break;
                }
                term *next = calloc(1, sizeof(term));
                t->right = next;
                next->left = parse(list);
                next->kind = FUN;
                t = next;
            }
            t->right = parse(list);
            t = start;
        }else{
            t->left = parse(list);
            if(accept(COLON_T, list)){
                replace_term(t, t->left);
                t->annotation = parse(list);
            }else if(accept(TO_T, list)){
                term *orig = t;
                t->kind = PI;
                t->left = convert_unnamed(t->left);
                t->right = parse(list);
                while(accept(TO_T, list)){
                    term *left = t->right;
                    left = convert_unnamed(left);
                    term *right = parse(list);
                    term *new = calloc(1, sizeof(term));
                    new->kind = PI;
                    new->left = left;
                    new->right = right;
                    t->right = new;
                    t = new;
                }
                t = orig;
            }else{
                t->kind = APP;
                t->right = parse(list);
            }
        }
        expect(CLOSE_T, list);
    }
    return t;
}

term *parse_term(char *s)
{
    token_list *tokens = tokenize(s);
    //print_tokens(tokens);
    token_list *start = tokens;
    term *t = parse(&tokens);
    free_tokens(start);
    debruijn(t);
    return t;
}

