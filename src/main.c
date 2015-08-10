#include <stdio.h>
#include <stdlib.h>
#include "term.h"
#include "environment.h"
#include "utils.h"
#include "parser.h"

int main(int argc, char **argv)
{
    int debug = 1;
    environment env = make_environment();
    while(1){
        if(debug){
            printf("~ ");
            fflush(stdout);
        }
        char *line = fgetl(stdin);
        if(!line) break;
        term *t = parse_term(line);
        if(debug){
            printf("Input: ");
            print_term(t);
            printf("\n");
        }
        evaluate_term(t, env);
        if(debug){
            printf("Output: ");
            print_term(t);
            printf("\n");
        }
        free_term(t);
        free(line);
    }

    return 0;
}

