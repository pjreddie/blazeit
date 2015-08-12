#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
    NONE_T, OPEN_T, CLOSE_T, FUN_T, VAR_T, ARR_T, TO_T, DEF_T, COLON_T, EQUAL_T, TYPE_T, UNDER_T
} token_kind;

typedef struct token_list{
    struct token_list *next;
    struct token_list *prev;
    token_kind kind;
    char *value;
} token_list;

token_list *tokenize(char *s);
void free_tokens(token_list *list);
void print_token(token_kind kind);
void print_tokens(token_list *list);
#endif
