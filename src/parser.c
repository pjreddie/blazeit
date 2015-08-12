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

