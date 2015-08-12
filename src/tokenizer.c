#include "tokenizer.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
            curr->value = copy_string("_");
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
                if(curr->prev){
                    fprintf(stderr, "Definitions must come first!");
                    return 0;
                }
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
        fprintf(stderr, "Didn't recognize char '%c'\n", *s);
        return 0;
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
