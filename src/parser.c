/*


ind nat : Type0 = 
| 0 : nat
| S : nat -> nat
.

def plus : (_:nat) -> (_:nat) -> nat =
    fun x => fun y =>
        nat.ind (fun _:nat => nat) y (fun _:nat => fun sum:nat => S sum) x.

fun x => x
            

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"

typedef enum {
    START_T, OPEN_T, CLOSE_T, FUN_T, VAR_T, ARR_T, TO_T
} token_kind;

typedef struct token_list{
    struct token_list *next;
    struct token_list *prev;
    token_kind kind;
    char *value;
} token_list;

token_list *tokenize(char *s)
{
    token_list *start = calloc(1, sizeof(token_list));
    start->kind = START_T;

    token_list *curr = start;
    token_list *next;
    //int len = strlen(s);
    while(s){
        next = calloc(1, sizeof(token_list));
        next->prev = curr;
        curr->next = next;
        curr = next;
        if (*s == '('){
            curr->kind = OPEN_T;
            curr->value = "(";
            ++s;
            continue;
        }
        if (*s == ')'){
            curr->kind = CLOSE_T;
            curr->value = ")";
            ++s;
            continue;
        }
        if (*s == 'f'){
            if (strncmp(s, "fun", 3) == 0){
                curr->kind = FUN_T;
                curr->value = "=";
                s += 3;
                continue;
            }
        }
        if (*s == '='){
            if (strncmp(s, "=>", 2) == 0){
                curr->kind = ARR_T;
                curr->value = "=>";
                s += 2;
                continue;
            }
        }
        if (*s == '-'){
            if (strncmp(s, "->", 2) == 0){
                curr->kind = TO_T;
                curr->value = "->";
                s += 2;
                continue;
            }
        }
        if (*s >= 65 && *s <= 90){
            int i = 0;
            while(s[i] >= 65 && s[i] <= 90) ++i;
            char *copy = calloc(i+1, sizeof(char));
            memcpy(copy, s, i);
            curr->kind = VAR_T;
            curr->value = copy;
            s += i;
        }
    }
    return start;
}

term *parse_term(char *s)
{
    term *t = calloc(1, sizeof(term));
    return t;
}


