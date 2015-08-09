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
    NONE_T, OPEN_T, CLOSE_T, FUN_T, VAR_T, ARR_T, TO_T
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
    }
}

typedef struct token_list{
    struct token_list *next;
    struct token_list *prev;
    token_kind kind;
    char *value;
} token_list;

int is_varchar(char s)
{
    return (s >= 65 && s <= 90) || (s >= 97 && s <= 122) || (s >= 48 && s <= 57);
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
            curr->value = "(";
            ++s;
            ++i;
            continue;
        }
        if (*s == ')'){
            curr->kind = CLOSE_T;
            curr->value = ")";
            ++s;
            ++i;
            continue;
        }
        if (*s == 'f'){
            if (strncmp(s, "fun", 3) == 0){
                curr->kind = FUN_T;
                curr->value = "=";
                s += 3;
                i += 3;
                continue;
            }
        }
        if (*s == '='){
            if (strncmp(s, "=>", 2) == 0){
                curr->kind = ARR_T;
                curr->value = "=>";
                s += 2;
                i += 2;
                continue;
            }
        }
        if (*s == '-'){
            if (strncmp(s, "->", 2) == 0){
                curr->kind = TO_T;
                curr->value = "->";
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

void expect(token_kind kind, token_list **l)
{
    token_kind k = (*l)->kind;
    if(k != kind){
        token_list *list = *l;
        while(list){
            printf("%s, ", list->value);
            list = list->next;
        }
        printf("\n");
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

term *parse(token_list **list)
{
    token_list *token = *list;
    term *t = calloc(1, sizeof(term));

    if (accept(VAR_T, list)){
        t->kind = VAR;
        t->name = token->value;
        return t;
    } else {
        expect(OPEN_T, list);
        if (accept(FUN_T, list)){
            t->kind = FUN;
            t->left = parse(list);
            expect(ARR_T, list);
            t->right = parse(list);
        }else{
            t->kind = APP;
            t->left = parse(list);
            t->right = parse(list);
        }
        expect(CLOSE_T, list);
    }
    return t;
}

term *parse_term(char *s)
{
    token_list *tokens = tokenize(s);
    term *t = parse(&tokens);
    debruijn(t);
    return t;
}


