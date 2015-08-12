#ifndef PARSER_H
#define PARSER_H
#include "term.h"
#include "tokenizer.h"

term *parse_string(char *s);
term *parse_term(token_list **list);
term *parse(token_list **list);

#endif
